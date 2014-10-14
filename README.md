OPTBOT
======

An option parser for shell scripts, based on liboptbot

Features
--------

Allows access to almost all liboptbot features from your shell scripts

* Parses long and short options
* Exports values given into environment variables when eval'd
* Automatic help generation
* Allows multiple uses of options, and access to all given values
* Allows access to parameters not parsed as flags or
  values to flags via argv and argc

Examples
--------

Warning: I haven't tested the example below yet.  If you need something that
runs (or is POSIX compliant) check out test.sh

```bash
eval `optbot \
  -a v,verbose,"enable verbose output",false \
  -a f,file,"add an output file",true \
  -a l,logfile,"The logfile to use.  Only the first given is utilized",true \
  -p cliopt_ -e \
  -- $@
`

[ -z "$cliopt_logfile" ] && cliopt_logfile=log.txt

echo "starting output" > $cliopt_logfile
[ "$cliopt_verbose" = true ] && echo "starting output"
# Do we have any files to operate on?
[ -z "$cliopt_file" ] && exit 0

# Iterate through files given with -f
seq 1 $cliopt_file_n | while read i; do
  optfile_var=cliopt_file_$i
  # This works in Bash.  Indirect expansion is much messier in leaner
  # shells.  See test.sh for an example.
  echo "Here's some output." >${!optfile_var}
done

# We can get arguments left over from the command line with the argc and argv
# variables as well!
#
# This will parse out args on the command line which are not picked up as
# flags or values passed to flags, or any arguments given after --
seq 1 $cliopt_argc | while read i; do
  argv_var=cliopt_argv_$i
  echo "argv number $i set to ${!argv_var}"
end

echo "done" > $cliopt_logfile
[ "$cliopt_verbose" = true ] && echo "done"
```

Calling this script with `./script.sh -f file_one.txt --file file_two.txt
-l script.log` is now possible.  calling it with `./script.sh --help` will print
a help page with the given arguments and descriptions.

Author
------

I'm Jack Forrest (jack@jrforrest.net).

Bugs
----

Send reports to bugs@jrforrest.net please.

THANK YOU AND ENJOY!
