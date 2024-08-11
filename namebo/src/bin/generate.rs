use anyhow::{Context, Result};
use clap::Parser;
use namebo::WordMap;
use std::fs::File;
use std::io::BufReader;

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Args {
    // "json file of letter counts"
    #[arg(short, long)]
    wordmap: String,
}

fn process(args: &Args) -> Result<()> {
    let path = &args.wordmap;
    let file = File::open(path).with_context(|| format!("unable to open file {:?}", path))?;
    let mut r = BufReader::new(file);
    let wordmap: WordMap = serde_json::from_reader(&mut r)?;
    println!("{}", wordmap.generate());
    Ok(())
}

fn main() {
    let args = Args::parse();
    match process(&args) {
        Ok(_) => (),
        Err(error) => panic!("{:?}", error),
    };
}
