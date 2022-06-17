const express = require('express');

const app = express(); // 생성자
var http = require('http');
const port = 3000;
app.use(express.json());
app.use(express.urlencoded({extended : false}));

var server = http.createServer(app);
var cnts = [0,0,0];
var all_cnt = 0;
var flag = 0;

app.get('/', (req, res) => {
    res.send("Hi! Ban!");
    console.log(`Ban`)
});

app.post('/update_data', (req, res) => {
    var lorry_no = req.body.lorry_no;
    var speed = req.body.speed;
    var distance = req.body.distance;
    // console.log(`Lorry No.${lorry_no} [speed:${speed}, distance:${distance}]`)
    if(!flag){
        for (let index = 0; index < cnts.length; index++) {
            if((index == lorry_no-1) && !cnts[index]){
                console.log(`Truck No.${lorry_no} \n Connected.`);
                cnts[index] = 1;
            }
            
            if(cnts[index])
                all_cnt++;
        }
        if((all_cnt == cnts.length) && !flag){
            console.log(`All Trucks Connected.`);
            flag = 1;
        }
    }else{
        console.log(`Truck No.${lorry_no} \n [speed:${speed}, distance:${distance}]`)
    }
    all_cnt = 0;
    // Distribute data according to Lorry No
    // res.json({speed:speed, distance:distance});
});

// app.post('/update_data', (req, res) => {
//     res.send("1234!!!!");
//     console.log(`def`)
// });

server.listen( port, () => {
    // console.log(`${port} Express server listen...`);
    console.log(`Platooning system operate.`);
});

server.keepAliveTimeout = 61 * 1000;
server.headersTimeout = 65 * 1000