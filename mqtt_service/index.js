import { createClient } from 'redis';

const redisClient = createClient();

redisClient.on('error', (err) => console.log('Redis Client Error', err));

await redisClient.connect();

await redisClient.set('key', 'value');
const value = await redisClient.get('key');
