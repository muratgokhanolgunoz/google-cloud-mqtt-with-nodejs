  'use strict';

  // Include libraries
  const {readFileSync} = require('fs')
  const jwt            = require('jsonwebtoken')
  const mqtt           = require('mqtt')
  var express          = require('express')
  var app              = express()

  // Information of the project we created on the Google Cloud Console page.
  const projectId          = '<YOUR PROJECT_ID>'
  const deviceId           = `<YOUR DEVICE_ID>`
  const registryId         = `<YOUR REGISTRY_ID>`
  const region             = `<YOUR REGION>`
  const algorithm          = `RS256`
  const privateKeyFile     = `./certs/rsa_private.pem`
  const serverCertFile     = `./certs/roots.pem`
  const mqttBridgeHostname = `mqtt.googleapis.com`
  const mqttBridgePort     = 8883
  const messageType        = `events`

  console.log('Google Cloud IoT Core MQTT')
  console.log("========================================================")

  // A token is required for the Google Cloud MQTT connection. It is the function that will create the token expression.
  const createJwt = (projectId, privateKeyFile, algorithm) => {
      const token = {
          iat: parseInt(Date.now() / 1000),
          exp: parseInt(Date.now() / 1000) + 20 * 60,
          aud: projectId,
      }

      const privateKey = readFileSync(privateKeyFile)
      return jwt.sign(token, privateKey, {algorithm: algorithm})
  };

  //Setting up the information required for the Google Cloud MQTT connection.
  const mqttClientId = `projects/${projectId}/locations/${region}/registries/${registryId}/devices/${deviceId}`

  let passwordJwt = createJwt(projectId, privateKeyFile, algorithm)
  const connectionArgs = {
      host: mqttBridgeHostname,
      port: mqttBridgePort,
      clientId: mqttClientId,
      username: 'unused',
      password: passwordJwt,
      protocol: 'mqtts',
      secureProtocol: 'TLSv1_2_method',
      ca: [readFileSync(serverCertFile)],
  }

  /*
  //  By using the GET method with the HTTP server, incoming requests are captured and the temperature and humidity data
  //  that the ardunion reads from the sensor are taken under this block, preparing the ground for the MQTT connection.
  */
  app.get('/iot', function (req, res) {
      console.log('Waiting for connection ...')
      const client = mqtt.connect(connectionArgs)

      client.subscribe(`/devices/${deviceId}/config`, {qos: 1})
      client.subscribe(`/devices/${deviceId}/commands/#`, {qos: 0})
      const mqttTopic = `/devices/${deviceId}/${messageType}`

      // MQTT connection is made with Google servers.
      client.on('connect', function(success) {
          if (success) {
              console.log('--- Client connected')
              sendData();
          } else {
              console.log('--- Client not connected')
          }
      })

      client.on('close', function() {
          console.log('--- Client closed');
          console.log("========================================================")
      })

      client.on('error', function(err) {
          console.log('Error : ', err)
      })

      function fetchData() {
          return {
              'temperature': req.query.temperature,
              'humidity'   : req.query.humidity,
              'time'       : new Date().toISOString().slice(0, 19).replace('T', ' ')
          }
      }

      function sendData() {
          var payload = fetchData();

          payload = JSON.stringify(payload)
          console.log('Publishing message : ', payload)

          // Temperature and humidity data are shared with Google server in JSON format.
          client.publish(mqttTopic, payload, { qos: 1 })
          client.end();
      }
  })

  // 127.0.0.1:3000 listening.
  app.listen(3000)
