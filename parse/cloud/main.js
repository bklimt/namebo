
var generate = require('cloud/generate');

Parse.Cloud.define("word", function(request, response) {
  response.success(generate());
});
