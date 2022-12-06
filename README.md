# fqsplit
fsplit is small C application for the efficient splitting of fastqs for parallel processing. fqsplit writes files in a round-robin process, depositing `--buffer-recs RECORDS` reads per round of distribution. For example, A consolidated fastq file conatining 25,000 records with the flags -n 5 -b 100 will result in 5 output files each containing 5,000 records. The first 100 records will go to the first file _1.basename.fastq -> the second 100 will go to _2.basename.fastq -> and so on.


Current version: 0.0.1

## Installation
```bash
git clone https://github.com/zwebbs/fqsplit.git
cd fqsplit/
make
```
## Usage
Usage details for fqsplit can be found using the `-h` or `--help` flags.

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
fqsplit works with fastq files piped from `stdin` as well. This is useful when you want to efficiently unzip and split
fastq file which are gzipped:
```bash
$ gunzip -c mysample.fastq.gz | ./fqsplit -s -n 4 -b 1000 mysample -

$ ls
1-of-4.mysample.fastq
2-of-4.mysample.fastq
3-of-4.mysample.fastq
4-of-4.mysample.fastq
mysample.fastq.gz
```


## Potential Improvements?
 * add gzip compression option to outputs?
 * performance improvements?
 * tests suite
