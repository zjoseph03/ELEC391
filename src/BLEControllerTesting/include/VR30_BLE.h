/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 #include <events/mbed_events.h>
 #include "ble/BLE.h"
 #include "pretty_printer.h"

 // Print BLE MAC address in standard format XX:XX:XX:XX:XX:XX
void printBLEAddress(const uint8_t* addr) {
  for (int i = 5; i >= 0; i--) {
      if (addr[i] < 16) Serial.print("0"); // Add leading zero
      Serial.print(addr[i], HEX);
      if (i > 0) Serial.print(":");
  }
}
 
 #if MBED_CONF_APP_FILESYSTEM_SUPPORT
 #include "LittleFileSystem.h"
 #include "HeapBlockDevice.h"
 #endif //MBED_CONF_APP_FILESYSTEM_SUPPORT
 
 /** This example demonstrates all the basic setup required for pairing and setting
  *  up link security both as a central and peripheral. It also demonstrates privacy
  *  features in Gap. It shows how to use private addresses when advertising and
  *  connecting and how filtering ties in with these operations.
  *
  *  The application will start by repeatedly trying to connect to the same
  *  application running on another board. It will do this by advertising and
  *  scanning for random intervals waiting until the difference in intervals
  *  between the boards will make them meet when one is advertising and the
  *  other scanning.
  *
  *  Two devices will be operating using random resolvable addresses. The
  *  applications will connect to the peer and pair. It will attempt bonding
  *  to store the IRK that resolve the peer. Subsequent connections will
  *  turn on filtering based on stored IRKs.
  */
 
 static const char DEVICE_NAME[] = "MOCUTE-052Fe-AUTO";
 
 using std::literals::chrono_literals::operator""ms;
 
 /* Delay between steps */
 static const std::chrono::milliseconds delay_duration = 3000ms;
 
 /** Base class for both peripheral and central. The same class that provides
  *  the logic for the application also implements the SecurityManagerEventHandler
  *  which is the interface used by the Security Manager to communicate events
  *  back to the applications. You can provide overrides for a selection of events
  *  your application is interested in.
  */
 class SecurityDemo : private mbed::NonCopyable<SecurityDemo>,
                      public SecurityManager::EventHandler,
                      public ble::Gap::EventHandler
 {
 public:
     SecurityDemo(BLE &ble, events::EventQueue &event_queue) :
         _ble(ble), _event_queue(event_queue) { };
 
     virtual ~SecurityDemo()
     {
         _ble.onEventsToProcess(nullptr);
     };
 
     /** Start BLE interface initialisation */
     void run()
     {
         /* this will inform us off all events so we can schedule their handling
          * using our event queue */
         _ble.onEventsToProcess(makeFunctionPointer(this, &SecurityDemo::schedule_ble_events));
 
         /* handle gap events */
         _ble.gap().setEventHandler(this);
 
         if (_ble.hasInitialized()) {
             /* ble instance already initialised, skip init and start activity */
             start();
         } else {
             ble_error_t error = _ble.init(this, &SecurityDemo::on_init_complete);
 
             if (error) {
              Serial.println();
              Serial.println("Error returned by BLE::init.\r\n");
              Serial.println();
              return;
             }
         }
 
         /* this will not return until shutdown */
         _event_queue.dispatch_forever();
     };
 
     /** Override to start chosen activity when the system starts */
     virtual void start() = 0;
 
     /* callbacks */
 
     /** This is called when BLE interface is initialised and starts the demonstration */
     void on_init_complete(BLE::InitializationCompleteCallbackContext *event)
     {
         ble_error_t error;
 
         if (event->error) {
             Serial.println("Error during the initialisation\r\n");
             return;
         }
 
         /* for use by tools we print out own address and also use it
          * to seed RNG as the address is unique */
         print_local_address();
 
 
     /* This path will be used to store bonding information but will fallback
          * to storing in memory if file access fails (for example due to lack of a filesystem) */
         const char* db_path = nullptr;
 
         error = _ble.securityManager().init(
             /* enableBonding */ false,
             /* requireMITM */ false,
             /* iocaps */ SecurityManager::IO_CAPS_NONE,
             /* passkey */ nullptr,
             /* signing */ false,
             /* dbFilepath */ nullptr
         );
 
         if (error) {
             Serial.println("Error during initialising security manager\r\n");
             return;
         }
 
         /* This tells the stack to generate a pairingRequest event which will require
          * this application to respond before pairing can proceed. Setting it to false
          * will automatically accept pairing. */
         _ble.securityManager().setPairingRequestAuthorisation(true);
 
 #if MBED_CONF_APP_FILESYSTEM_SUPPORT
         error = _ble.securityManager().preserveBondingStateOnReset(true);
 
         if (error) {
             print_error(error, "Error during preserveBondingStateOnReset %d\r\n");
         }
 #endif // MBED_CONF_APP_FILESYSTEM_SUPPORT
 
         /* this demo switches between being master and slave */
         _ble.securityManager().setHintFutureRoleReversal(true);
 
         /* Tell the security manager to use methods in this class to inform us
          * of any events. Class needs to implement SecurityManagerEventHandler. */
         _ble.securityManager().setSecurityManagerEventHandler(this);
 
         /* gap events also handled by this class */
         _ble.gap().setEventHandler(this);
 
         error = _ble.gap().enablePrivacy(true);
         if (error) {
             Serial.println("Error enabling privacy.\r\n");
             return;
         }
 
         /* continuation is in onPrivacyEnabled() */
     };
 
     /** Schedule processing of events from the BLE in the event queue. */
     void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context)
     {
         _event_queue.call([&ble_instance = context->ble] { ble_instance.processEvents(); });
     };
 
 private:
     /* SecurityManager Event handler */
 
     /** Respond to a pairing request. This will be called by the stack
      * when a pairing request arrives and expects the application to
      * call acceptPairingRequest or cancelPairingRequest */
     void pairingRequest(ble::connection_handle_t connectionHandle) override
     {
         Serial.println("Pairing requested - authorising\r\n");
         _ble.securityManager().acceptPairingRequest(connectionHandle);
     }
 
     /** Inform the application of pairing */
     void pairingResult(
         ble::connection_handle_t connectionHandle,
         SecurityManager::SecurityCompletionStatus_t result
     ) override
     {
         if (result == SecurityManager::SEC_STATUS_SUCCESS) {
             Serial.println("Pairing successful\r\n");
             _bonded = true;
         } else {
             Serial.println("Pairing failed\r\n");
             Serial.println((int)result);
         }
 
         _event_queue.call_in(
             delay_duration,
             [this, connectionHandle] {
                 _ble.gap().disconnect(connectionHandle, ble::local_disconnection_reason_t::USER_TERMINATION);
             }
         );
     }
 
     /** Inform the application of change in encryption status. This will be
       * communicated through the serial port */
     void linkEncryptionResult(ble::connection_handle_t connectionHandle, ble::link_encryption_t result) override
     {
         if (result == ble::link_encryption_t::ENCRYPTED) {
            Serial.println("Link ENCRYPTED\r\n");
         } else if (result == ble::link_encryption_t::ENCRYPTED_WITH_MITM) {
            Serial.println("Link ENCRYPTED_WITH_MITM\r\n");
         } else if (result == ble::link_encryption_t::NOT_ENCRYPTED) {
            Serial.println("Link NOT_ENCRYPTED\r\n");
         }
     }
 
     void onPrivacyEnabled() override
     {
         /* all initialisation complete, start our main activity */
         start();
     }
 
    protected:
     /* Gap Event handler */
 
     /** This is called by Gap to notify the application we connected */
     void onConnectionComplete(const ble::ConnectionCompleteEvent &event) override
     {
         Serial.println("Connected to peer: ");
         printBLEAddress(event.getPeerAddress().data());
         if (event.getPeerResolvablePrivateAddress() != ble::address_t()) {
            Serial.println("Peer random resolvable address: ");
            printBLEAddress(event.getPeerResolvablePrivateAddress().data());
         }
 
         _handle = event.getConnectionHandle();
 
         if (_bonded) {
             /* disconnect in 2s */
             _event_queue.call_in(
                 delay_duration,
                 [this] {
                     _ble.gap().disconnect(_handle, ble::local_disconnection_reason_t::USER_TERMINATION);
                 }
             );
         } else {
             /* start bonding */
             ble_error_t error = _ble.securityManager().setLinkSecurity(
                 _handle,
                 SecurityManager::SECURITY_MODE_ENCRYPTION_NO_MITM
             );
             if (error) {
                 Serial.println("Failed to set link security\r\n");
                 _ble.gap().disconnect(_handle, ble::local_disconnection_reason_t::USER_TERMINATION);
             }
         }
     };
 
     /** This is called by Gap to notify the application we disconnected */
     void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) override
     {
         if (_bonded) {
             /* we have connected to and bonded with the other device, from now
              * on we will use the second start function and stay in the same role
              * as peripheral or central */
             Serial.println("Disconnected.\r\n\r\n");
             _event_queue.call_in(delay_duration, [this] { start(); });
         } else {
              Serial.println("Failed to bond.\r\n");
             _event_queue.break_dispatch();
         }
     };
 
     void onScanTimeout(const ble::ScanTimeoutEvent &) override
     {
         /* if we failed to find the other device, abort so that we change roles */
         Serial.println("Haven't seen other device, switch modes.\r\n");
         _event_queue.break_dispatch();
     }
 
     void onAdvertisingEnd(const ble::AdvertisingEndEvent &event) override
     {
         if (!event.isConnected()) {
             Serial.println("No device connected to us, switch modes.\r\n");
             _event_queue.break_dispatch();
         }
     }
 
 private:
     void print_local_address()
     {
         /* show what address we are using now */
         ble::own_address_type_t addr_type;
         ble::address_t addr;
         _ble.gap().getAddress(addr_type, addr);
         Serial.println("Device address: ");
         printBLEAddress(addr.data());
         static bool _seeded = false;
         if (!_seeded) {
             _seeded = true;
             /* use the address as a seed */
             uint8_t* random_data = addr.data();
             srand(*((unsigned int*)random_data));
         }
     }
 
 protected:
     BLE &_ble;
     events::EventQueue &_event_queue;
     ble::connection_handle_t _handle = 0;
     bool _bonded = false;
 };
 
 
 /** A central device will scan and connect to a peer. */
 class VR30Central : public SecurityDemo {
 public:
     VR30Central(BLE &ble, events::EventQueue &event_queue)
         : SecurityDemo(ble, event_queue) { }
 
     /** start scanning and attach a callback that will handle advertisements
      *  and scan requests responses */
     void start() override
     {
         ble::central_privacy_configuration_t privacy_configuration = {
             /* use_non_resolvable_random_address */ false,
             ble::central_privacy_configuration_t::DO_NOT_RESOLVE
         };
         if (_bonded) {
             Serial.println("We are bonded - we will only see known devices\r\n");
             privacy_configuration.resolution_strategy = ble::central_privacy_configuration_t::RESOLVE_AND_FILTER;
         }
 
         _ble.gap().setCentralPrivacyConfiguration(&privacy_configuration);
 
         start_scanning();
     }
 
     /* helper functions */
 private:
     bool start_scanning()
     {
         ble_error_t error;
         ble::ScanParameters scan_params;
         _ble.gap().setScanParameters(scan_params);
 
         _is_connecting = false;
 
         if (_bonded) {
             /* if we bonded it means we have found the other device, from now on
              * wait at each step until completion */
             error = _ble.gap().startScan(ble::scan_duration_t::forever());
         } else {
             /* otherwise only scan for a limited time before changing roles again
              * if we fail to find the other device */
             error = _ble.gap().startScan(ble::scan_duration_t::forever());
         }
 
         if (error) {
             Serial.println("Error during Gap::startScan\r\n");
             return false;
         }
 
         Serial.println("Scanning...\r\n");
 
         return true;
     }
 
 private:
     /* Event handler */
 
     /** Look at scan payload to find a peer device and connect to it */
     void onAdvertisingReport(const ble::AdvertisingReportEvent &event) override
     {
         /* don't bother with analysing scan result if we're already connecting */
         if (_is_connecting) {
             return;
         }
 
         ble::AdvertisingDataParser adv_data(event.getPayload());
 
         /* parse the advertising payload, looking for a discoverable device */
         while (adv_data.hasNext()) {
             ble::AdvertisingDataParser::element_t field = adv_data.next();
 
             /* connect to a known device by name */
             if (field.type == ble::adv_data_type_t::COMPLETE_LOCAL_NAME &&
                 field.value.size() == strlen(DEVICE_NAME) &&
                 (memcmp(field.value.data(), DEVICE_NAME, field.value.size()) == 0)) {
 
                  Serial.println("We found a connectable device: \r\n");
                  printBLEAddress(event.getPeerAddress().data());
 
                 ble_error_t error = _ble.gap().stopScan();
 
                 if (error) {
                  Serial.println("Error caused by Gap::stopScan");
                     return;
                 }
 
                 error = _ble.gap().connect(
                     event.getPeerAddressType(),
                     event.getPeerAddress(),
                     ble::ConnectionParameters()
                 );
 
                 Serial.println("Connecting...\r\n");
 
                 if (error) {
                  Serial.println("Error caused by Gap::connect");
                     return;
                 }
 
                 /* we may have already scan events waiting
                  * to be processed so we need to remember
                  * that we are already connecting and ignore them */
                 _is_connecting = true;
 
                 return;
             }
         }
     }
     
    void onConnectionComplete(const ble::ConnectionCompleteEvent &event) override {
      SecurityDemo::onConnectionComplete(event);
      
      // After connection, discover HID service
      if (event.getStatus() == BLE_ERROR_NONE) {
        Serial.println("Connected to controller, discovering services...\r\n");
          _ble.gattClient().onServiceDiscoveryTermination(
              makeFunctionPointer(this, &VR30Central::onServiceDiscoveryTermination)
          );
          
          _ble.gattClient().launchServiceDiscovery(
              _handle, 
              nullptr,  // No specific service UUID
              nullptr   // No specific characteristic UUID
          );
      }
    }
  
  void onServiceDiscoveryTermination(ble::connection_handle_t handle) {
    // Find the HID service and set up notifications
    setupHIDService(handle);
  }
  
  void setupHIDService(ble::connection_handle_t handle) {
      // Code to find HID service by UUID "1812"
      // Then set up notifications on report characteristic "2A4D"
      // ...
  }
  
  void processHIDReport(const uint8_t* data, size_t len) {
      // Process controller input and update motors
      // ...
  }
 
 private:
     bool _is_connecting = false;
 };
 
 #if MBED_CONF_APP_FILESYSTEM_SUPPORT
 bool create_filesystem()
 {
     static LittleFileSystem fs("fs");
 
     /* replace this with any physical block device your board supports (like an SD card) */
     static HeapBlockDevice bd(4096, 256);
 
     int err = bd.init();
 
     if (err) {
         return false;
     }
 
     err = bd.erase(0, bd.size());
 
     if (err) {
         return false;
     }
 
     err = fs.mount(&bd);
 
     if (err) {
         /* Reformat if we can't mount the filesystem */
         printf("No filesystem found, formatting...\r\n");
 
         err = fs.reformat(&bd);
 
         if (err) {
             return false;
         }
     }
 
     return true;
 }
 #endif //MBED_CONF_APP_FILESYSTEM_SUPPORT

