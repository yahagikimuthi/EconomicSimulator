FROM node:22-slim

WORKDIR /app

RUN npm install -g pnpm

RUN pnpm install

RUN pnpm add recharts
RUN pnpm add @mui/material @emotion/react @emotion/styled @mui/icons-material

EXPOSE 5173

CMD ["pnpm", "dev", "--host"]