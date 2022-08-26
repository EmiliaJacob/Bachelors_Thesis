const express = require('express');
const ydb = require('nodem').Ydb();

const app = express();
app.use(express.json());

app.listen(4000);


app.post('/deactivateArticle', (req, res) => {
  // if(!(req.body.global && req.body.subscripts[0]==''){ TODO: Is this check neccessary?
  //   res.status(400).send('wrong data');
  //   return;
  // }

  reqArticleId = req.body.articleId;

  ydb_data_result = ydb.data({global: "articles", subscripts: [reqArticleId]}); 

  if(ydb_data_result.defined == 0) {
    res.status(400).send("article not found");
    return;
  }

  activeData = ydb.get({global: "articles", subscripts: [reqArticleId, "active"]}).data;

  if(activeData == 0) {
    res.status(400).send('article already inactice');
    return;
  }

  ydb.set({global: 'articles', subscripts:[reqArticleId, 'active'], data:0}, (error, result) => {
    if(error){
      res.status(500).send(error);
      return;
    }
    res.status(200).send("successfully deactivated article");
  });
});

app.post('/set', (req, res) => {
  reqGlobal = req.body.global;
  reqData = req.body.data;
  if(req.body.subscripts) {
    reqSubscripts = req.body.subscripts;
    ydb.set({global: reqGlobal, subscripts: reqSubscripts, data: reqData}, (error, result) => {
      if(error){
        res.status(500).send(error);
        return;
      }
      res.status(200).send(`successfully set: ${reqGlobal}(${reqSubscripts})=${reqData}\n`);
    });
  }
  else {
    ydb.set({global: reqGlobal, data: reqData}, (error, result) => {
      if(error){
        res.status(500).send(error);
        return;
      }
      res.status(200).send(`successfully set: ${reqGlobal}=${reqData}\n`);
    });
  }

});

ydb_status = ydb.open(
  {
    routinesPath: '/app/node_modules/nodem/src',
    callinTable: '/app/node_modules/nodem/resources/nodem.ci'
  }
);

console.log(ydb_status);

ydb.set({global: 'ot', subscripts: ['active'], data: 1});
ydb.set({global: 'ot', subscripts: ['price'], data: 700});
