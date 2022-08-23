const express = require('express');
const ydb = require('nodem').Ydb();

const app = express();
app.use(express.json());

app.listen(4000);

app.post('/expired', (req, res) => {

  ydb.set({global: 'ot', subscripts: ['active'], data: 0});
  ydb.set({global: 'ot', subscripts: ['price'], data: 700});

  // check recieved JSON
  if(!req.body.articleName){
    res.status(400).send('wrong data');
    return;
  }

  ydb_data_result = ydb.data({global: req.body.articleName}); 

  // check if article with said name exists. people could otherwise spam the db
  if(ydb_data_result.defined == 0) {
    res.status(400).send("article not found");
    return;
  }

  // set active property to false which actives a trigger in ydb
  ydb.set({global: req.body.articleName, subscripts:['active'], data:0}, (error, result) => {
    if(error){
      res.status(500).send('ydb_error');
      return;
    }
    res.status(200).send("successfully updated property");
  });
});

ydb_status = ydb.open(
  {
    routinesPath: '/app/node_modules/nodem/src',
    callinTable: '/app/node_modules/nodem/resources/nodem.ci'
  }
);

console.log(ydb_status);
