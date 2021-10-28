# Pretty-printers for the Unitex C++ NLP Core (https://unitexgramlab.org)
# https://github.com/UnitexGramLab/unitex-core
# cristian.martinez@unitexgramlab.org (martinec)

# C++ related beautifiers (optional)
# @see https://stackoverflow.com/a/5713387
# @see https://gist.github.com/skyscribe/3978082

#set print pretty on
#set print object on
#set print static-members on
#set print vtbl on
#set print demangle on
#set demangle-style gnu-v3
#set print sevenbit-strings off
#set follow-fork-mode child
#set detach-on-fork off

# setup python environment
# mkdir -p ~/sources
# ln -s /your/git/folder/unitex-core ~/sources/unitex-core
set python print-stack full
python
import sys, os
sys.path.insert(1, os.getenv('HOME') + '/sources/unitex-core/misc/gdb')
import unitex
unitex.register_printers(unitex_version=(4,0,0))
end

