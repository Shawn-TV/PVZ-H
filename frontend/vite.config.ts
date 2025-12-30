import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [react()],
  // 使用相对路径，确保Electron加载本地文件时正常工作
  base: './',
  server: {
    port: 5173,
    open: false  // Electron模式下不需要自动打开浏览器
  },
  build: {
    outDir: 'dist',
    sourcemap: true,
    // 确保资源使用相对路径
    assetsDir: 'assets'
  },
  optimizeDeps: {
    exclude: ['lucide-react'],
  },
});
