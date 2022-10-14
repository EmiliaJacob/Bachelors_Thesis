const ydb = require('nodem').Ydb();

const express = require('express');
const app = express();
app.use(express.json());
app.listen(4000);

app.post('/deactivateArticle', (req, res) => {
  let reqArticleId = req.body.articleId;

  let ydb_data_result = ydb.data({global: 'articles', subscripts: [reqArticleId]}); 

  if(ydb_data_result.defined == 0) {
    res.status(400).send('article not found');
    return;
  }

  let articleActiveStatus = ydb.get({global: 'articles', subscripts: [reqArticleId, 'active']}).data;

  if(articleActiveStatus == 0) {
    res.status(400).send('article already inactice');
    return;
  }

  ydb.set({global: 'articles', subscripts:[reqArticleId, 'active'], data: 0}, (error, result) => {
    if(error){
      res.status(500).send(error);
      return;
    }
    res.status(200).send('successfully deactivated article');
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

console.log(`ydb_rest_service started with status: ${ydb_status}`);