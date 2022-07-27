PS1='${debian_chroot:+($debian_chroot)}\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ '
#export HISTSIZE=2000
alias ls="ls --color=auto"
alias ll="ls -l --color=auto"
alias dog="pygmentize -g -P bg=dark"

alias vbp='vi /storage/brno12-cerit/home/zikesfil/.bashrc'
alias sbp='source /storage/brno12-cerit/home/zikesfil/.bashrc'
alias cbp='cd /storage/brno12-cerit/home/zikesfil/'
alias cb3='cd /storage/brno3-cerit/home/zikesfil/'
alias qu='qstat -u zikesfil'
alias qn='qstat -u zikesfil'
#PATH=$PATH:/storage/brno12-cerit/home/zikesfil/.local/bin
export PATH=$PATH:/storage/brno12-cerit/home/zikesfil/scripts

#cd_modulefiles() {
#    cd /afs/.ics.muni.cz/packages/amd64_linux26/modules-2.0/modulefiles
#}
if [[ -f ~/.cluster_functions ]]; then
    source ~/.cluster_functions
fi


