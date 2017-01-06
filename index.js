const Particle = require('particle-api-js');
const Winston = require('winston');

const Config = require('./config');

Winston.add(Winston.transports.File, { filename: 'events.log' });
const particle = new Particle();
let token = null;

particle.login({username: Config.get('USERNAME'), password: Config.get('PASSWORD')}).then(
  (data) => {
    console.log('API call completed on promise resolve: ', data.body.access_token);
    token = data.body.access_token;
    listen();
  },
  (err) => {
    console.log('API call completed on promise fail: ', err);
  }
);

function listen() {
  particle.getEventStream({ deviceId: 'mine', auth: token }).then((stream) => {
    console.log('Started listening');
    stream.on('event', (data) => {
      Winston.log('info', data);
    });
  }, (err) => {
    console.log('Failed to start listening: ', err);
  });
}
