import { useEffect, useRef, useState } from 'react';
import Phaser from 'phaser';
import { GameScene } from '../scenes/GameScene.js';
import { NetworkClient } from '../network/client.js';

interface GameContainerProps {
  onBack: () => void;
  isMultiplayer?: boolean;
}

export function GameContainer({ onBack, isMultiplayer = false }: GameContainerProps) {
  const gameContainerRef = useRef<HTMLDivElement>(null);
  const gameRef = useRef<Phaser.Game | null>(null);
  const networkClientRef = useRef<NetworkClient | null>(null);
  const [isLoading, setIsLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [isPaused, setIsPaused] = useState(false);
  const [isGameOver, setIsGameOver] = useState(false);

  useEffect(() => {
    let mounted = true;

    const initGame = async () => {
      if (!gameContainerRef.current) return;

      try {
        console.log('连接到服务器...');

        // Create network client
        const networkClient = new NetworkClient('ws://localhost:8080');
        networkClientRef.current = networkClient;

        // Connect to server
        await networkClient.connect();
        console.log('已连接到服务器');

        // 发送RESTART_GAME消息启动新游戏（确保每次都是新游戏）
        // 传入multiplayer标志，让后端立即启用玩家控制模式（防止AI种植）
        networkClient.send('RESTART_GAME', { multiplayer: isMultiplayer });
        console.log('已发送RESTART_GAME消息，启动新游戏，多人模式:', isMultiplayer);

        if (!mounted) {
          networkClient.disconnect();
          return;
        }

        // Create Phaser game config - matching game.js settings
        const config: Phaser.Types.Core.GameConfig = {
          type: Phaser.AUTO,
          width: window.innerWidth,
          height: window.innerHeight,
          backgroundColor: '#1a4d1a',
          parent: gameContainerRef.current,
          scale: {
            mode: Phaser.Scale.FIT,
            autoCenter: Phaser.Scale.CENTER_BOTH
          },
          physics: {
            default: 'arcade',
            arcade: {
              gravity: { x: 0, y: 0 },
              debug: false
            }
          },
          scene: [GameScene]
        };

        // Create Phaser game instance
        const game = new Phaser.Game(config);
        gameRef.current = game;

        // Start the game scene with network client and multiplayer flag
        console.log('启动GameScene, isMultiplayer:', isMultiplayer);
        game.scene.start('GameScene', { networkClient, isMultiplayer });

        // 监听游戏场景事件
        game.events.once('ready', () => {
          const scene = game.scene.getScene('GameScene');
          if (scene) {
            // 监听返回主菜单事件
            scene.events.on('returnToMenu', () => {
              onBack();
            });
            // 监听游戏结束事件
            scene.events.on('gameOver', () => {
              setIsGameOver(true);
            });
          }
        });

        // 延迟设置事件监听，确保场景已创建
        setTimeout(() => {
          const scene = game.scene.getScene('GameScene');
          if (scene) {
            scene.events.on('returnToMenu', () => {
              onBack();
            });
          }
        }, 500);

        setIsLoading(false);
        console.log('游戏启动成功！');
      } catch (err) {
        console.error('游戏启动失败:', err);
        if (mounted) {
          setError('无法连接到服务器。请确保游戏服务器正在运行。');
          setIsLoading(false);
        }
      }
    };

    initGame();

    // Cleanup on unmount
    return () => {
      mounted = false;

      if (gameRef.current) {
        gameRef.current.destroy(true);
        gameRef.current = null;
      }

      if (networkClientRef.current) {
        networkClientRef.current.disconnect();
        networkClientRef.current = null;
      }
    };
  }, [onBack, isMultiplayer]);

  // Handle ESC key to pause/unpause (only when game is not over)
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      // 游戏结束后不响应暂停
      if (isGameOver) return;

      if (e.key === 'Escape') {
        setIsPaused(prev => {
          const newPaused = !prev;
          // Pause or resume the game scene
          if (gameRef.current) {
            const scene = gameRef.current.scene.getScene('GameScene');
            if (scene) {
              if (newPaused) {
                scene.scene.pause();
              } else {
                scene.scene.resume();
              }
            }
          }
          // Send pause/resume to backend
          if (networkClientRef.current) {
            networkClientRef.current.send(newPaused ? 'PAUSE' : 'RESUME', {});
          }
          return newPaused;
        });
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [isGameOver]);

  const handleResume = () => {
    setIsPaused(false);
    if (gameRef.current) {
      const scene = gameRef.current.scene.getScene('GameScene');
      if (scene) {
        scene.scene.resume();
      }
    }
    // Send resume to backend
    if (networkClientRef.current) {
      networkClientRef.current.send('RESUME', {});
    }
  };

  const handleBackToMenu = () => {
    setIsPaused(false);
    onBack();
  };

  if (error) {
    return (
      <div className="min-h-screen bg-gray-900 flex flex-col items-center justify-center text-white">
        <div className="text-center">
          <p className="text-xl mb-4 text-red-400">{error}</p>
          <button
            onClick={onBack}
            className="px-6 py-3 bg-gray-600 hover:bg-gray-500 rounded text-lg transition-colors"
          >
            返回主菜单
          </button>
        </div>
      </div>
    );
  }

  return (
    <div className="min-h-screen bg-gray-900 relative">
      {isLoading && (
        <div className="loading text-white">
          加载游戏中...
        </div>
      )}

      {/* Game container */}
      <div
        id="game-container"
        ref={gameContainerRef}
        className="w-screen h-screen flex items-center justify-center"
      />

      {/* HUD */}
      <div id="hud" style={{ display: 'none' }}>
        <div id="health-bar">
          <span>生命值: </span>
          <div className="bar">
            <div className="fill" id="health-fill" style={{ width: '100%' }}></div>
          </div>
        </div>
        <div id="inventory">
          {/* Inventory items will be added dynamically */}
        </div>
      </div>

      {/* Pause Menu */}
      {isPaused && (
        <div className="fixed inset-0 bg-black/70 flex items-center justify-center z-50">
          <div className="bg-gray-800 p-8 rounded-lg shadow-2xl text-center">
            <h2 className="text-3xl font-bold text-white mb-8">游戏暂停</h2>
            <div className="space-y-4">
              <button
                onClick={handleResume}
                className="w-full px-8 py-3 bg-green-600 hover:bg-green-500 text-white text-lg font-medium rounded transition-colors"
              >
                继续游戏
              </button>
              <button
                onClick={handleBackToMenu}
                className="w-full px-8 py-3 bg-gray-600 hover:bg-gray-500 text-white text-lg font-medium rounded transition-colors"
              >
                返回主菜单
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
