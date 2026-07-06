import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

export default defineConfig({
  plugins: [react()],
  server: {
    host: '0.0.0.0',
    port: 5173,
    proxy: {
      '/npc': 'http://localhost:8080',
      '/world': 'http://localhost:8080',
      '/evidence': 'http://localhost:8080',
      '/board': 'http://localhost:8080'
    }
  }
});
