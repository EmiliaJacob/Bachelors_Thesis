const express = require('express');
const app = express();
const port = 4000;

app.use(express.json());

app.set('views', './views');
app.set('view engine', 'pug');

app.listen(port);


app.get('/', (req,res) => {
    res.render('index', {title: 'Hey', message: 'Hello world'});
})