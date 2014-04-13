
var map = require('cloud/map');

var choose = function(data) {
  var sum = 0;
  for (var suffix in data) {
    sum = sum + data[suffix];
  }

  var r = Math.random() * sum;
  for (var suffix in data) {
    r -= data[suffix];
    if (r <= 0) {
      return suffix;
    }
  }

  return '?';
};

var generate = function() {
  var word = "^";
  while (word[word.length - 1] !== '$' && word[word.length - 1] !== '?') {
    var data = map[word.substr(-3)];
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

module.exports = generate;

