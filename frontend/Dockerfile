FROM node:20-alpine

WORKDIR /app

# Viteのデフォルト開発ポート
EXPOSE 5173

# ホストOSからの接続を許可して開発サーバーを起動
CMD ["sh", "-c", "npm install && npm run dev -- --host"]