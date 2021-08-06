# mutation_finder
Finds mutations in a FASTA file (assumes first sequence is the reference sequence) and outputs in a viewable manner

# Requirements
Developed/tested in Ubuntu 20.04, compiled with gcc. This uses getline() thus is likely not going to work on systems where that is not supported.

# Instructions
INPUTFILE expects a fasta file.

```./mutations INPUTFILE OUTPUTFILE```

e.g.

```./mutations test1.fa test1.out```

# Output
Gives an output file (filepath is OUTPUTFILE) in the same format as INPUTFILE but with non-mutations replaced with a '-' character. Deletions were specifically requested to be padded with a '-' character.
Gives an output csv file (e.g. if OUTPUTFILE was 'test1.out', this file would be 'test1.csv') that provides the identifier followed by each nucleotide of the sequence separated by a comma. Non-mutations are replaced with a '-' character. Deletions were specifically requested to NOT be padded with a '-' character.
Gives another output csv file (e.g. if OUTPUTFILE was 'test1.out', this file would be 'test1_list.csv') that provides the identifier and a mutation in N#M (where N is reference nucleotide, # is the index of the reference nucleotide, and M is the mutation) separated by a comma for each mutation.
Examples of the input/output files can be found in the tests folder.

# Note:
The file inputs I was given assumes that the newline is denoted by 2 characters.
If your newline is denoted by a single '\n' character, you will need to edit the lines that are similar to 
```string[nchar-2] = '\0';```
to
```string[nchar-1] = '\0';```

# Runtime:
The slowest runtime step is the dynamic programming step for calculating the string editing. For each pair of strings, this should occur in O(n * m) time, where n and m are the lengths of the strings. For S strings, this then makes the runtime O(S * n * m) time.
