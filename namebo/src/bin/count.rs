use anyhow::{Context, Result};
use clap::Parser;
use serde::Serialize;
use std::collections::HashMap;
use std::fs::File;
use std::io::{BufRead, BufReader};

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Args {
    // "file of words to read in"
    #[arg(short, long)]
    input: String,
}

#[derive(Debug, Serialize)]
struct MapEntry {
    total: i32,
    next: HashMap<char, i32>,
}

impl MapEntry {
    fn new() -> MapEntry {
        MapEntry {
            total: 0,
            next: HashMap::new(),
        }
    }

    fn add(&mut self, next: char) {
        self.total += 1;
        if !self.next.contains_key(&next) {
            self.next.insert(next, 0);
        }
        let current: &mut i32 = self.next.get_mut(&next).unwrap();
        *current += 1;
    }
}

#[derive(Debug, Serialize)]
struct WordMap {
    total: i32,
    entries: HashMap<String, MapEntry>,
}

impl WordMap {
    fn new() -> WordMap {
        WordMap {
            total: 0,
            entries: HashMap::new(),
        }
    }

    fn add_suffix(&mut self, prefix: &str, c: char) {
        // println!("adding suffix {:?} for {:?}", c, prefix);
        self.total += 1;
        if !self.entries.contains_key(prefix) {
            self.entries.insert(prefix.to_string(), MapEntry::new());
        }
        self.entries.get_mut(prefix).unwrap().add(c)
    }

    fn add_word(&mut self, word: &str) {
        let mut previous: Vec<usize> = Vec::new();
        for (i, c) in word.char_indices() {
            let (seen, _) = word.split_at(i);
            if previous.len() >= 3 {
                // trigram
                let j = previous[previous.len() - 3];
                let (_, prefix) = seen.split_at(j);
                self.add_suffix(prefix, c);
            }
            if previous.len() >= 2 {
                // bigram
                let j = previous[previous.len() - 2];
                let (_, prefix) = seen.split_at(j);
                self.add_suffix(prefix, c);
            }
            if previous.len() >= 1 {
                // unigram
                let j = previous[previous.len() - 1];
                let (_, prefix) = seen.split_at(j);
                self.add_suffix(prefix, c);
            }
            previous.push(i);
        }
    }
}

fn is_valid_char(c: char) -> bool {
    if c.is_ascii_alphabetic() {
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
        let line = line.to_lowercase();
        let word = format!("^{}$", line);

        map.add_word(&word);
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
