import { createServer, IncomingMessage, Server, ServerResponse } from 'http';

//import * as express from 'express';
import * as http from 'http';

var webServer:Server;
var webServerPort = 80;

var url = require('url');

import * as path from 'path';

function startServer()
{
	//create a server object:
	webServer = createServer(serverReq);
	
	webServer.listen(webServerPort); //the server object listens on port 8080
    console.log("server running at:" + webServerPort);
}

function serverReq(req:IncomingMessage, res:ServerResponse)
{
	res.setHeader("Access-Control-Allow-Origin", "*");
	if (req.method == 'GET')
		serverReq_GET(req, res);
	else if (req.method == 'POST')
		serverReq_POST(req,res);
}

function serverReq_GET(req:IncomingMessage, res:ServerResponse)
{
	//var q = url.parse(req.url, true).query;
	
	res.writeHead(200, {'Content-Type': 'text/html'});
    res.write('unknown command' + req.url); //write a response to the client
	res.end(); //end the response
}

export function serverReq_POST(req:IncomingMessage, res:ServerResponse)
{
	const chunks:Uint8Array[] = [];
	
	res.writeHead(200, {'Content-Type': 'text/html'});
	res.write('OK'); //write a response to the client
	res.end(); //end the response
	req.on('data', (chunk) => {chunks.push(chunk)});
	req.on('end', () => {
        console.log(chunks);
		var jsonString = Buffer.concat(chunks).toString();
		console.log('POST req data:\n' + jsonString);
		//var jsonObj= JSON.parse(jsonString);
		
	})
}

startServer();