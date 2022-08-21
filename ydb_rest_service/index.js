const express = require('express');
const ydb = require('nodem').Ydb();

const app = express();
app.use(express.json());

app.listen(4000);
app.post('/expired', (req, res) => {
  // check recieved JSON
  if(!req.body.articleName){
    res.status(400).send('wrong data');
    return;
  }
  // check if article with said name exists. people could otherwise spam the db
  res.send('hi' + data({global: req.body.articleName}));
  ydb.set({global: req.body.articleName, subscripts:['active'], data:0}, (error, result) => {
    if(error){
      res.status(500).send('ydb_error');
      return;
    }
    res.status(200).send("hi");
  });
});

ydb_status = ydb.open(
  {
    routinesPath: '/app/node_modules/nodem/src',
    callinTable: '/app/node_modules/nodem/resources/nodem.ci'
  }
);

console.log(ydb_status);
