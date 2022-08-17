# syntax=docker/dockerfile:1

FROM node:18
ENV NODE_ENV=production
WORKDIR /app
COPY . .
# COPY ["package.json", "package-lock.json*", "./"]
RUN npm install --production
CMD ["node", "index.js"]
