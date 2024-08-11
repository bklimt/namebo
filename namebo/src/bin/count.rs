use anyhow::{Context, Result};
use clap::Parser;
use namebo::WordMap;
use std::fs::File;
use std::io::{BufRead, BufReader};

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Args {
    // "file of words to read in"
    #[arg(short, long)]
    input: String,
}

fn is_valid_char(c: char) -> bool {
    if c.is_ascii_lowercase() {
        return true;
    }
    if c == '\'' {
        return true;
    }
    return false;
}

fn is_word(s: &str) -> bool {
    if s.is_empty() {
        return false;
    }
    if s.chars().nth(0).unwrap().is_uppercase() {
        return false;
    }
    if !s.chars().all(is_valid_char) {
        return false;
    }
    if s.ends_with("'s") {
        return false;
    }
    return true;
}

fn process(args: &Args) -> Result<()> {
    let path = &args.input;
    let file = File::open(path).with_context(|| format!("unable to open file {:?}", path))?;
    let mut r = BufReader::new(file);

    let mut map = WordMap::new();

    loop {
        let mut line = String::new();
        let n = r.read_line(&mut line).unwrap();
        let line = line.trim();

        if line == "" {
            if n == 0 {
                break;
            }
            continue;
        }

        if !is_word(line) {
            continue;
        }
        // println!("{}", line);
        // let line = line.to_lowercase();
        let word = format!("^{}$", line);

        map.add_word_str(&word);
    }

    println!("{}", serde_json::to_string(&map)?);

    Ok(())
}

fn main() {
    let args = Args::parse();
    match process(&args) {
        Ok(_) => (),
        Err(error) => panic!("{:?}", error),
    };
}
