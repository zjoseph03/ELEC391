# ELEC391
Documentation for Working on Features in Git
This document outlines the process for working on features in this project using Git. Follow these steps to ensure that your work is properly tracked and integrated into the main project.

1. Create a New Branch for Each Feature
When starting to work on a new feature, you should create a new branch specifically for that feature. This keeps the work isolated and avoids disrupting the main codebase.

Command to create and switch to a new branch:
bash
Copy code
git checkout -b feature/feature-name
Replace feature-name with a descriptive name for your feature (e.g., feature/login-page, feature/new-ui, etc.).
This command creates a new branch and automatically switches to it.
2. Commit Regularly
As you work on your feature, make sure to commit your changes regularly with clear, concise commit messages. This allows for better tracking of progress and easier rollbacks if necessary.

Commands for adding and committing changes:
Add changes to the staging area:
bash
Copy code
git add .
This adds all changes (new files, modified files, etc.) to the staging area.
Commit the changes:
bash
Copy code
git commit -m "Descriptive commit message"
Replace "Descriptive commit message" with a meaningful message that explains the changes you made.
3. Push Your Feature Branch to GitHub
Once you've made several commits and want to back up your work or share it with others, push your feature branch to GitHub. This ensures your work is available remotely.

Command to push the branch to GitHub:
bash
Copy code
git push -u origin feature/feature-name
This pushes your local feature branch to the remote repository on GitHub.
The -u flag sets the remote branch as the default for this local branch, so future git push commands can be used without specifying the branch name.
4. Test the Feature Thoroughly
Before you consider merging the feature into the main branch, ensure that it works fully as expected. Test your code locally or in a development environment. Only proceed to the next step when you are confident that the feature is functioning properly.

5. Merge the Feature into main
Once the feature is complete and fully tested, it is time to merge it into the main branch. First, make sure you are on the main branch and that it is up to date.

Steps to merge the feature into main:
Switch to main branch:

bash
Copy code
git checkout main
Pull the latest changes from GitHub to ensure your main is up to date:

bash
Copy code
git pull origin main
Merge your feature branch into main:

bash
Copy code
git merge feature/feature-name
Replace feature/feature-name with the name of your feature branch.
Resolve any merge conflicts (if any):

If there are conflicts, Git will mark them in the affected files. You will need to manually resolve these conflicts and then commit the resolved changes.
6. Push the Updated main Branch to GitHub
After merging, push the updated main branch to GitHub so others can access the latest changes.

Command to push main to GitHub:
bash
Copy code
git push origin main
7. Delete the Feature Branch (Optional)
Once the feature has been merged, you can delete the local feature branch and the remote branch to keep the repository clean.

Command to delete the local feature branch:
bash
Copy code
git branch -d feature/feature-name
Command to delete the remote feature branch:
bash
Copy code
git push origin --delete feature/feature-name
Summary of Commands
Create and switch to a new feature branch:

bash
Copy code
git checkout -b feature/feature-name
Add changes to the staging area:

bash
Copy code
git add .
Commit the changes:

bash
Copy code
git commit -m "Descriptive commit message"
Push the feature branch to GitHub:

bash
Copy code
git push -u origin feature/feature-name
Switch to the main branch:

bash
Copy code
git checkout main
Pull the latest changes to main:

bash
Copy code
git pull origin main
Merge your feature into main:

bash
Copy code
git merge feature/feature-name
Push the updated main branch:

bash
Copy code
git push origin main
Delete the local and remote feature branch (optional):

Local branch:
bash
Copy code
git branch -d feature/feature-name
Remote branch:
bash
Copy code
git push origin --delete feature/feature-name
