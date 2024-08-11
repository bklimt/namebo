
import { SuffixScores, WORD_MAP } from './map.js';

function choose(data: SuffixScores): string {
  let sum = 0;
  for (let suffix in data) {
    sum = sum + data[suffix];
  }

  let r = Math.random() * sum;
  for (var suffix in data) {
    r -= data[suffix];
    if (r <= 0.0001) {
      return suffix;
    }
  }

  return '?';
};

export function generate(): string {
  let word = "^";
  while (word[word.length - 1] !== '$' && word[word.length - 1] !== '?') {
    const data = WORD_MAP[word.substr(-3)];
    word = word + choose(data);
  }
  if (word[0] === '^') {
    word = word.substr(1);
  }
  if (word[word.length - 1] === '$') {
    word = word.substr(0, word.length - 1);
  }
  return word;
};
