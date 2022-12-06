# fqsplit
small C application to efficiently split fastqs for parallel processing

Current version: 0.0.1

## Installation
```bash
git clone https://github.com/zwebbs/fqsplit.git
cd fqsplit/
make
```
## Usage
```bash
$ fqsplit -h

./fqsplit usage:
./fqsplit [OPTIONS] OUTPUT_BASENAME INPUT_FASTQ
-----------------------------------------------

arguments:
-n, --n-splits			Number of files to split INPUT FASTQ into
-b, --buffer-recs		Number of records to write before rotating between output files
-s, --smk-format		Prefix style for output files should be in the snakemake scatter style (etc. 1-of-n.)
-o, --outdir			Directory in which to place output files
OUTPUT_BASENAME			File prefix for output fastqs (should not conatin suffix (i.e.e .fastq)
INPUT_FASTQ				Fastq file to split among the outputs. (-) signifies piping from stdin

```


## Potential Improvements?
 * add gzip compression option to outputs?
 * performance improvements?
 * tests suite
