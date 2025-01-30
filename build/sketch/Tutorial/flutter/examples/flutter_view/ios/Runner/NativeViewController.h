#line 1 "d:\\Courses\\ELEC391\\BluetoothTesting\\Tutorial\\flutter\\examples\\flutter_view\\ios\\Runner\\NativeViewController.h"
// Copyright 2014 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <UIKit/UIKit.h>


@protocol NativeViewControllerDelegate <NSObject>

- (void)didTapIncrementButton;

@end

@interface NativeViewController: UIViewController
@property (strong, nonatomic) id<NativeViewControllerDelegate> delegate;
- (void) didReceiveIncrement;
@end
