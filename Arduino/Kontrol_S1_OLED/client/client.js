const SerialPort = require('serialport')
const Readline = require('@serialport/parser-readline')

const serialport = new SerialPort('/dev/tty.usbmodem10118501')

const parser = serialport.pipe(new Readline({
  delimiter: '\n'
}));

serialport.on("open", () => {
  console.log('serial port open')
});
parser.on('data', data => {
  console.log('got word from arduino:', data)
  // get char codes
  // console.log([...data].map(c => c.charCodeAt(0)))
});

const port = 8080
// SerialPort.list().then(
//     ports => ports.forEach(console.log),
//     err => console.error(err))

const express = require('express')
const bodyParser = require('body-parser')
const app = express()

const infoMap = {
  'title': 0,
  'artist': 1,
  'bpm': 2,
  'length': 3,
  'progress': 4
}

app.use(bodyParser.json())

app.post('/deckLoaded/A', function (request, response) {
  console.log(request.body)
  sendInfo('artist', request.body.artist)
    .then(() => sendInfo('artist', request.body.artist))
    .then(() => sendInfo('bpm', request.body.bpm.toFixed(2)))
    .then(() => sendInfo('title', request.body.title))
    .then(() => sendInfo('length', Math.floor(request.body.trackLength)))
  // isSynced:
  response.writeHead(200, {
    'Content-Type': 'text/html'
  })
  response.end('ok')
})

app.post('/updateDeck/A', function (request, response) {
  if (request.body.elapsedTime) sendInfo('progress', Math.floor(request.body.elapsedTime))
  response.writeHead(200, {
    'Content-Type': 'text/html'
  })
  response.end('ok')
})

app.listen(port)
console.log(`Listening at http://localhost:${port}`)

function sendInfo(infoTitle, info) {
  return new Promise((res, rej) =>  {
    const infoString = String(info)
    const typeByte = infoMap[infoTitle]
    const infoLength = infoString.length - 1 + 1
    const data = Buffer.from([
      typeByte,
      infoLength,
      ...[...infoString].map(c => c.charCodeAt(0)),
      '\0'
    ])
    console.log(`Writing: "${infoTitle}" (${typeByte}) with value: ${infoString} (${infoLength})`)
    // console.log(...[...infoString].map(c => c.charCodeAt(0)))
    serialport.write(data, (err) => err ? rej(err) : res())
  })
}
