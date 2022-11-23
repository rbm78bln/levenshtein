# levenshtein

A grep-like command line utlity for fuzzy string matching based on the [Levenshtein](https://www.google.com/search?q=Vladimir+Levenshtein&oq=Vladimir+Levenshtein&ie=UTF-8) [distance](https://en.wikipedia.org/wiki/Levenshtein_distance)

```
$ levenshtein -h

usage: levenshtein {-(c delim|C delim|i|g|u|a maxd|r maxd|R maxd|v|s order|
   S order|H count|T count|d|D|b|m type)} (-h|[-f source|string1] string2)

where:
   string1/2 : Strings between the Levenshtein-distance is calculated.
               Only the resulting distance will be displayed.
               Returns 0, iff the result is within the given
               filter-ranges (see below). Returns 1 if not.

   -f source : Read string1 from file 'source'.
               Then compare line by line with string2.
               Every line from 'source' that is not filtered (see below)
               will be printed to stdout in the given sorting order.
               If the result is not sorted by distance (see below) every
               line will be preceeded by its distance to 'string2' by
               default. This behaviour can be overridden by -d or -D.
               Iff at least one line matches the given filter-ranges,
               0 is returned. Returns 1 if none matches.

               If string1 and -f are omitted, then stdin is used.

  preprocessing:
   -c delim  : Only use those parts of both strings that appear after
               the last occurence of 'delim'. If 'delim' does not appear
               in one string, then don't cut anything from the beginning.

   -C delim  : Only use those parts of both strings that appear before
               the first occurence of 'delim'. If 'delim' does not appear
               in one string, then don't cut anything from the end.

   -i        : ignore case.

   -g        : grep style. Match 'string2' with any substring of 'string1'
               or lines from 'source'. This may take some time!

   -w        : Match only with one ore more whole words. Implies -g. This
               often gives more reasonable results and works much faster.

  filtering:
   -u        : unfiltered. (default) Clear all previously defined filters. Any
               line will be printed and the result code will always be 0.

   -a maxd   : absolute distance. Only distances less or equal to 'maxd'
               are considered to be valid and will result in a return code
               of 0. All lines with distances above 'maxd' will be suppressed.

   -r maxd   : relative distance to length of 'string1' or 'source' in percent.
               Same as -a, but 'maxd' is determined by the length of 'string1'
               or the length of the current line from 'source'. E.g. if
               'string1' has a length of 10 characters and 'maxd' is given 30%,
               then the maximum valid Levenshtein-distance would be 3.
               If used with -g, then 'maxd' is relative to the currently
               checked substring, not the whole string!

   -R maxd   : relative distance to length of 'string2' in percent.
               Same as -r, but 'maxd' referes to the length of 'string2'.

               -r and -R are taken from the length of the corresponding
               strings AFTER preprocessing!

   -v        : inverse. All filters are inverted. Result codes too.

  sorting:     only for use with -f.
   -s order  : Sorts the results by the given order ascendingly.
   -S order  : Sorts the results by the given order descendingly.
               where 'order' is one of:
                  u : unsorted   Does not reorder items. (default)
                  d : distance   Orders items according to their distance.
                  a : alpha      Orders items alphabetically.
                  r : randomize  Reorders items by chance.

  limiting:    only for use with -f.
   -H count  : Only display the first 'count' lines of the resulting list.
   -T count  : Only display the last 'count' lines of the resulting list.
               Both can be combined and even repeated, but mind their order!
               e.g. -H 2 -T 1 displays the second best result.

  format:      only for use with -f.
   -d        : Force preceeding any output line with its distance to 'string2'.
   -D        : Prevent preceeding any output line with its distance.

  more format: implies -g.
   -b        : Print the byte-range of the best matching substrings as 2nd item.
   -m        : Print only the best matches instead of the whole line.
   -h        : Displays this cruft.
```

# How to build a GNU Make based project

```shell
# Build project
$ make

# Run program
$ ./levenshtein -h

# Clean build files
$ make clean
```

# How to build PlatformIO based project

1. [Install PlatformIO Core](https://docs.platformio.org/page/core.html)
2. Run these commands:

```shell
# Build project
$ pio run

# Run program
$ .pio/build/linux_x86_64/program -h

# Clean build files
$ pio run --target clean
```
![-](https://miunske.eu/github/?levenshtein)
