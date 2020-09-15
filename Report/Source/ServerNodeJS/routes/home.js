module.exports = function (app) {
    app.get('/register', function (req, res) {
        res.render('register');
    });

    app.get('/home', function (req, res) {
        res.render('home');
    });

    app.post('/open', function (req, res) {
        console.log("aaaaaa");
        var fs = require('fs');
        fs.writeFile('./file/data.txt', 1, (err) => {
            if (err) return console.log(err);
        });

    });

    app.post('/close', function (req, res) {
        var fs = require('fs');
        fs.writeFile('./file/data.txt', 0, (err) => {
            if (err) return console.log(err);
        });

    });

    app.get('/openDoor', function (req, res) {
        var fs = require('fs');
        fs.readFile('./file/data.txt', 'utf8', (err, contents) => {
            res.json(parseInt(contents));
            res.status = 200;
        });
    });

    app.post('/log', function (req, res) {
        var fs = require('fs');
        var data = req.body.time;
        console.log(req.body);
        fs.writeFile('./file/log.txt', data, (err) => {
            if (err) return console.log(err);
            res.status = 200;
        });
    });
}