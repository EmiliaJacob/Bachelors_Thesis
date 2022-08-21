const express = require('express');
const ydb = require('nodem').Ydb();

const app = express();
app.use(express.json());

app.listen(4000);

app.post('/expired', (req, res) => {
  if(!req.body.articleName){
    res.status(400).send('wrong data');
    return;
  }
  ydb.set({global: req.body.articleName, subscripts:['active'], data:0}, (error, result) => {
    if(error){
      res.status(500).send('ydb_error");
      return;
    }
    res.status(200).send(result);
  });
  else
    res.send('idk');
});

ydb_status = ydb.open(
  {
    routinesPath: '/app/node_modules/nodem/src',
    callinTable: '/app/node_modules/nodem/resources/nodem.ci'
  }
);

console.log(ydb_status);
