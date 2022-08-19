const express = require('express');
const ydb = require('nodem').Ydb();

const app = express();

app.listen(4000);

app.get('/', () => {
  console.log("ydb service works great");
  res.send('Hello World');
});

ydb_status = ydb.open(
  {
    routinesPath: '/app/node_modules/nodem/src',
    callinTable: '/app/node_modules/nodem/resources/nodem.ci'
  }
);

console.log(ydb_status);
