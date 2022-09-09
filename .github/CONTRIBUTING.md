## Contributing

> Unitex/GramLab project decision-making is based on a community meritocratic process. Anyone with an interest in Unitex/GramLab can join the community, contribute to the project design and participate in decisions. See http://unitexgramlab.org/how-to-contribute for more detailed information on how to start contributing to Unitex/GramLab.

You are welcome to contribute by [forking this repository](https://help.github.com/articles/fork-a-repo/)
and [sending pull requests](https://help.github.com/articles/using-pull-requests/)
with your changes. The recommended [workflow](http://rypress.com/tutorials/git/rebasing) to contribute is:

1. [Fork us](https://github.com/UnitexGramLab/unitex-core/fork)

1. **Clone** your fork locally

    ```
    git clone https://github.com/YOUR_GITHUB_USERNAME/unitex-core.git
    ```

1. Configure the **upstream** remote. To do this, add the remote location of the main
   `unitex-core` repository under the name `upstream`. This will allow you later
   to keep your fork up to date

    ```
    git remote add upstream git://github.com/UnitexGramLab/unitex-core.git
    git fetch upstream
    ```
 
 1. Create a local **branch** for your changes

    If your new branch depends on a branch that has not yet been merged, use the following:

    ```
    git checkout branch-not-yet-merged
    git branch my-branch branch-not-yet-merged
    ```

    Otherwise:
    ```
    git checkout -b my-branch origin/master
    ```

   Use a short and descriptive name for your branch. If you are developing a new
   feature or enhancement, name your branch as `feature/DESCRIPTIVE-NAME`, if
   you are fixing a bug, name your branch as `fix/N` where `N` corresponds to
   an [issue number](https://github.com/UnitexGramLab/unitex-core/issues),
   e.g. `fix/5`

1. For non-trivial changes, if it doesn't already exist, create a
   [**new issue**](https://github.com/UnitexGramLab/unitex-core/issues/new)
  
1. Edit the files and **compile** your code following the *How to Build* instructions

    - Refrain from using new C++ libraries, so as not to limit the ability to compile code under architectures, platforms or other C/C++ compilers that are incompatible with them. (e.g. instead of vector use struct list_pointer)

1. Execute [`./unitex-core-test.sh -p1 -M1`](https://github.com/UnitexGramLab/unitex-core-tests#getting-started)
   to run non-regression and memory error detection **tests**. Note that is not necessary or even
   recommended to fork the [unitex-core-tests](https://github.com/UnitexGramLab/unitex-core-tests) repository.
   In this case, since you need only to run the `unitex-core-test.sh` script, make sure only your local repository
   is up-to-date by [pulling](https://help.github.com/articles/fetching-a-remote/#pull) the latest
   remote changes

1. Make sure git knows your name and email address, e.g.

    ```
    git config --global user.name "John Doe"
    git config --global user.email "john.doe@example.org"
    ```

1. If changes made include new files, add them in files src/build/Unitex4_vs2019.*

1. **Commit** your code following the [Commit Message Guidelines](https://github.com/UnitexGramLab/unitex-doc-contributor-guidelines/blob/master/pages/05.guidelines/01.commits/docs.md)

1. Make sure your fork is **up to date**

    ```
    git checkout master
    git pull upstream master
    ```

1. [**Rebase**](https://www.atlassian.com/git/tutorials/rewriting-history/git-rebase-i) your local branch

    ```
    git checkout my-changes
    git rebase master
    ```

1. **Push** your changes to your remote repository on GitHub

    ```
    git push origin
    ```

1. Go to ``https://github.com/YOUR_GITHUB_USERNAME/unitex-core`` and [Request a pull](https://github.com/UnitexGramLab/unitex-core/pulls)

1. Give a brief description and [refer the issues](https://help.github.com/articles/autolinked-references-and-urls/#issues-and-pull-requests)
   in your pull request comment

1. Finally, if your are developing or improving a new functionality or module, you can,
   and should, contribute tests for it. To get further details check the
   [unitex-core-tests](https://github.com/UnitexGramLab/unitex-core-tests) repository
