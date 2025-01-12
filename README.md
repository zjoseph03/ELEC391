### ELEC 391


# Documentation for Working on Features in Git

This document outlines the process for working on features in this project using Git. Follow these steps to ensure that your work is properly tracked and integrated into the main project.

---

### **1. Create a New Branch for Each Feature**
When starting to work on a new feature, you should create a new branch specifically for that feature. This keeps the work isolated and avoids disrupting the main codebase.

#### **Command to create and switch to a new branch:**
```bash
git checkout -b branch-name

```
#### Commit Frequently
As you work on your feature, make sure to commit your changes regularly with clear, concise commit messages. This allows for better tracking of progress and easier rollbacks if necessary.

### **2. Commit Regularly
As you work on your feature, make sure to commit your changes regularly with clear, concise commit messages. This allows for better tracking of progress and easier rollbacks if necessary.

Commands for adding and committing changes:
- Add changes to the staging area:
  - Adding all changes under the current directory to be staged (NOT PUSHED ANYWHERE):
    - git add .
- Commit your changes to your local repo BUT NOT the remote github repository
  - git commit -m "Descriptive commit message"


### **3 Pushing your feature branch to github
Do this everytime you've used git add . on your local repo: 
  - git push -u origin branch-name




### **4. After Testing to make sure nothing goes wrong add to the main branch
- Switch to main branch:
  - git checkout main
- Pull the latest changes from github to make sure your repo is not outdated:
  - git pull origin main
- Merge your feature branch into main:
  - git merge branch-name
- Resolve any conflicts that may arise
- Test again after resolving


### **5. After testing with the merged code to main on your local repo push to github
 - git push origin main


### Optional: Deleting a feature branch
Delete your local feature branch: 
 - git branch -d branch-name
Delete the github feature branch: 
 - git push origin --delete branch-name


### **Working and pushing directly to main
If you create your changes on your local repo while on the main branch and you want to push directly to main use the following commands: 
git add your-folder/file-name
git commit -m "Added your-folder-name with relevant contents"
git push origin main


