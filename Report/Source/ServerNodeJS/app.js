var express = require('express');
var app = express();
var path = require('path');
var mongoose = require("mongoose");

var bodyParser = require('body-parser');
var multer = require('multer');
const WebSocket = require('ws');
const WS_PORT = 8888;
const HTTP_PORT = 8000;

global.dbHelper = require('./common/dbHelper');

global.db = mongoose.connect("mongodb://127.0.0.1:27017/test1");

app.set('views', path.join(__dirname, 'views'));


app.set('view engine', 'html');
app.engine('.html', require('ejs').__express);

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
app.use(multer());

app.use(express.static(path.join(__dirname, 'public')));

const wsServer = new WebSocket.Server({ port: WS_PORT }, () => console.log(`WS Server is listening at ${WS_PORT}`));
let connectedClients = [];
wsServer.on('connection', (ws, req) => {
    console.log('Connected');
    connectedClients.push(ws);

    ws.on('message', data => {
        connectedClients.forEach((ws, i) => {
            if (ws.readyState === ws.OPEN) {
                ws.send(data);
            } else {
                connectedClients.splice(i, 1);
            }
        })
    });
});

require('./routes')(app);

app.get('/', function (req, res) {
    res.render('home');
});

app.listen(HTTP_PORT);


