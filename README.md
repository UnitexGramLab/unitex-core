# Unitex/GramLab Core [![Build Status](https://travis-ci.org/UnitexGramLab/unitex-core.svg?branch=master)](https://travis-ci.org/UnitexGramLab/unitex-core) [![Build status](https://ci.appveyor.com/api/projects/status/ucuu8ocibexmagju?svg=true)](https://ci.appveyor.com/project/martinec/unitex-core)

Unitex is the Natural Language Processing (NLP) engine of [Unitex/GramLab][unitexgramlab].

## How to Build

    git clone https://github.com/UnitexGramLab/unitex-core
    cd unitex-core/build
    make DEBUG=yes UNITEXTOOLLOGGERONLY=yes

## Contributing

We welcome everyone to contribute to improve the Unitex Core by [forking this repository](https://help.github.com/articles/fork-a-repo/)
and [sending pull requests](https://help.github.com/articles/using-pull-requests/)
with their changes. The recommended [workflow](http://rypress.com/tutorials/git/rebasing) to contribute is:

1. [Fork us](https://github.com/UnitexGramLab/unitex-core/fork)

1. Clone your fork locally

    ```
    git clone https://github.com/YOUR_GITHUB_USERNAME/unitex-core.git
    ```

1. Create a local branch for your changes

    ```
    git checkout -b my-changes master
    ```

1. For non-trivial changes, if it doesn't already exist, create a
   [new issue](https://github.com/UnitexGramLab/unitex-core/issues/new)

1. Edit files and compile your code following the *How to Build* instructions above

1. Execute [`./unitex-core-test.sh -M1`](https://github.com/UnitexGramLab/unitex-core-tests#getting-started)
   to run non-regression and memory error detection tests. Note that is not necessary or even
   recommended to fork the [unitex-core-tests](https://github.com/UnitexGramLab/unitex-core-tests) repository.
   In this case, since you need only to run the `unitex-core-test.sh` script, make sure only your local repository
   is up-to-date by [pulling](https://help.github.com/articles/fetching-a-remote/#pull) the latest
   remote changes

1. Commit your code referring in the [commit message](https://help.github.com/articles/closing-issues-via-commit-messages) the issue you worked on

1. Make sure your fork is up to date

    ```
    git checkout master
    git pull upstream
    ```

1. [Rebase](https://www.atlassian.com/git/tutorials/rewriting-history/git-rebase-i) your local branch

    ```
    git checkout my-changes
    git rebase master
    ```

1. Merge back into master

    ```
    git checkout master
    git merge my-changes
    ```

1. Push your changes to your remote repository on GitHub

    ```
    git push
    ```

1. [Request a pull](https://github.com/UnitexGramLab/unitex-core/pulls)

1. Give a brief description and [refer the issues](https://help.github.com/articles/autolinked-references-and-urls/#issues-and-pull-requests)
   in your pull request comment

1. Finally, if your are developing or improving a new functionality or module, you can,
   and should, contribute tests for it. To get further details check the
   [unitex-core-tests](https://github.com/UnitexGramLab/unitex-core-tests) repository
    
## Documentation

User's Manual (in PDF format) is available in English and French ([more
translations are welcome](https://github.com/UnitexGramLab/unitex-doc-usermanual)).
You can view and print them with Evince, downloadable
[here](https://wiki.gnome.org/Apps/Evince/Downloads).
The latest version of the User's Manual is accessible
[here](http://releases.unitexgramlab.org/latest-stable/man/).

## Support

Support questions can be posted in the [community support
forum](http://forum.unitexgramlab.org). Please feel free to submit any
suggestions or requests for new features too. Some general advice about
asking technical support questions can be found
[here](http://www.catb.org/esr/faqs/smart-questions.html).

## Reporting Bugs

See the [Bug Reporting
Guide](http://unitexgramlab.org/index.php?page=6) for information on
how to report bugs.

## Governance Model

Unitex/GramLab project decision-making is based on a community
meritocratic process, anyone with an interest in it can join the
community, contribute to the project design and participate in
decisions. The [Unitex/GramLab Governance
Model](http://governance.unitexgramlab.org) describes
how this participation takes place and how to set about earning merit
within the project community.

## Spelling

Unitex/GramLab is spelled with capitals "U" "G" and "L", and with
everything else in lower case. Excepting the forward slash, do not put
a space or any character between words. Only when the forward slash
is not allowed, you can simply write “UnitexGramLab”.

It's common to refer to the Unitex/GramLab Core as "Unitex", and to the
Unitex Project-oriented IDE as "GramLab". If you are mentioning the
distribution suite (Core, IDE, Linguistic Resources and others bundled
tools) always use "Unitex/GramLab".


## License

<a href="/LICENSE"><img height="48" align="left" src="http://www.gnu.org/graphics/empowered-by-gnu.svg"></a>

This program is licensed under the [GNU Lesser General Public License version 2.1](/LICENSE). 
Contact unitex-devel@univ-mlv.fr for further inquiries.

--

Copyright (C) 2016 Université Paris-Est Marne-la-Vallée

[unitexgramlab]:  http://unitexgramlab.org
