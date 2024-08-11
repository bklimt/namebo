use itertools::Itertools;
use rand::random;
use serde::{Deserialize, Serialize};
use std::collections::{BTreeMap, HashMap};

#[derive(Debug, Serialize, Deserialize)]
struct MapEntry(BTreeMap<char, i32>);

impl MapEntry {
    fn new() -> MapEntry {
        MapEntry(BTreeMap::new())
    }

    fn add(&mut self, next: char) {
        *self.0.entry(next).or_insert(0) += 1;
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub struct WordMap(BTreeMap<String, MapEntry>);

impl WordMap {
    pub fn new() -> WordMap {
        WordMap(BTreeMap::new())
    }

    fn add_suffix_str(&mut self, prefix: &str, c: char) {
        // println!("adding suffix {:?} for {:?}", c, prefix);
        if !self.0.contains_key(prefix) {
            self.0.insert(prefix.to_string(), MapEntry::new());
        }
        self.0.get_mut(prefix).unwrap().add(c)
    }

    fn add_suffix_slice(&mut self, prefix: &[char], c: char) {
        let prefix: String = prefix.iter().collect();
        self.add_suffix_str(&prefix, c);
    }

    pub fn add_word(&mut self, word: &str) {
        let mut previous: Vec<usize> = Vec::new();
        for (i, c) in word.char_indices() {
            let (seen, _) = word.split_at(i);
            if previous.len() >= 3 {
                // trigram
                let j = previous[previous.len() - 3];
                let (_, prefix) = seen.split_at(j);
                self.add_suffix_str(prefix, c);
            }
            if previous.len() >= 2 {
                // bigram
                let j = previous[previous.len() - 2];
                let (_, prefix) = seen.split_at(j);
                self.add_suffix_str(prefix, c);
            }
            if previous.len() >= 1 {
                // unigram
                let j = previous[previous.len() - 1];
                let (_, prefix) = seen.split_at(j);
                self.add_suffix_str(prefix, c);
            }
            previous.push(i);
        }
    }

    pub fn add_word_slice(&mut self, word: &[char]) {
        for (i, c) in word.iter().enumerate() {
            if i >= 3 {
                // trigram
                self.add_suffix_slice(&word[i - 3..i], *c);
            }
            if i >= 2 {
                // bigram
                self.add_suffix_slice(&word[i - 2..i], *c);
            }
            if i >= 1 {
                // unigram
                self.add_suffix_slice(&word[i - 1..i], *c);
            }
        }
    }

    pub fn add_word_str(&mut self, word: &str) {
        let chars: Vec<char> = word.chars().collect_vec();
        self.add_word_slice(&chars);
    }

    fn collect_weights(
        &self,
        p: &mut HashMap<char, f32>,
        total: &mut f32,
        previous: &[char],
        len: usize,
        weight: f32,
    ) {
        if previous.len() < len {
            return;
        }
        let j = previous.len();
        let i = j - len;
        let key = &previous[i..j];
        let key: String = key.iter().collect();
        let Some(table) = self.0.get(&key) else {
            return;
        };
        let mut table_total = 0i32;
        for (_, n) in table.0.iter() {
            table_total += *n;
        }
        for (c, n) in table.0.iter() {
            let w = ((*n as f32) * (table_total as f32)) * weight;
            *total += w;
            *p.entry(*c).or_insert(0.0) += w;
        }
    }

    fn next_letter(&self, previous: &[char]) -> char {
        // Compute the probabilities.
        let mut p: HashMap<char, f32> = HashMap::new();
        let mut total: f32 = 0.0;

        if previous.len() >= 3 {
            self.collect_weights(&mut p, &mut total, previous, 3, 1.0);
        } else if previous.len() >= 2 {
            self.collect_weights(&mut p, &mut total, previous, 2, 1.0);
        } else {
            self.collect_weights(&mut p, &mut total, previous, 1, 1.0);
        }

        let mut r = random::<f32>() * total;
        for (c, w) in p.iter() {
            r -= *w;
            if r <= 0.00001 {
                return *c;
            }
        }

        let ps = serde_json::to_string_pretty(&p).unwrap_or_else(|e| format!("invalid: {}", e));
        panic!("invalid {} in {}", r, ps);
    }

    pub fn generate(&self) -> String {
        let mut word = vec!['^'];
        loop {
            let c = self.next_letter(&word);
            word.push(c);
            if c == '$' {
                break;
            }
            if word.len() > 100 {
                word.push('$');
                break;
            }
        }
        word[1..word.len() - 1].iter().collect()
    }
}
