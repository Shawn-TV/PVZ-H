import { Globe, User, X } from 'lucide-react';
import { useState } from 'react';

// 多语言文本
const LANGUAGES = {
  zh: {
    title: 'PLANTS VS. ZOMBIES',
    subtitle: 'MAZE EDITION',
    singlePlayer: '单人游戏',
    multiPlayer: '多人游戏',
    howToPlay: '玩法介绍',
    howToPlayTitle: '游戏玩法介绍',
    howToPlayContent: [
      '【游戏目标】',
      '僵尸：穿越迷宫，找到出口逃离！',
      '戴夫：种植植物，阻止僵尸逃跑！',
      '',
      '【操作按键】',
      '单人模式：',
      '  WASD/方向键 - 控制僵尸移动',
      '  Ctrl - 使用撑杆跳跃障碍',
      '  Tab/Shift - 打开小地图',
      '',
      '多人模式：',
      '  WASD - 控制戴夫移动',
      '  方向键 - 控制僵尸移动',
      '  Q - 打开种植菜单',
      '  Ctrl - 撑杆跳跃',
      '',
      '【道具系统】',
      '🪣 铁桶 - 增加护甲值，抵御攻击',
      '🏃 撑杆跳 - 可跳过一格障碍物',
      '❤️ 生命药水 - 恢复40%生命值',
      '⚡ 速度药水 - 移动速度提升50%',
      '',
      '【植物介绍】',
      '🌱 豌豆射手 - 向僵尸发射豌豆',
      '🌱 双发射手 - 一次发射两颗豌豆',
      '🥜 坚果墙 - 阻挡僵尸前进',
      '🍒 樱桃炸弹 - 范围爆炸秒杀僵尸'
    ],
    options: '选项...',
    exitGame: '退出游戏',
    languageTitle: '选择语言',
    chinese: '中文',
    english: 'English',
    close: '关闭'
  },
  en: {
    title: 'PLANTS VS. ZOMBIES',
    subtitle: 'MAZE EDITION',
    singlePlayer: 'Single Player',
    multiPlayer: 'Multiplayer',
    howToPlay: 'How to Play',
    howToPlayTitle: 'How to Play',
    howToPlayContent: [
      '【OBJECTIVE】',
      'Zombie: Navigate the maze and escape!',
      'Dave: Plant defenses to stop the zombie!',
      '',
      '【CONTROLS】',
      'Single Player:',
      '  WASD/Arrows - Move zombie',
      '  Ctrl - Pole vault jump',
      '  Tab/Shift - Open minimap',
      '',
      'Multiplayer:',
      '  WASD - Move Dave',
      '  Arrows - Move zombie',
      '  Q - Open plant menu',
      '  Ctrl - Pole vault jump',
      '',
      '【ITEMS】',
      '🪣 Bucket - Adds armor protection',
      '🏃 Pole Vault - Jump over one obstacle',
      '❤️ Health Potion - Restore 40% HP',
      '⚡ Speed Potion - 50% speed boost',
      '',
      '【PLANTS】',
      '🌱 Peashooter - Shoots peas at zombies',
      '🌱 Repeater - Fires two peas at once',
      '🥜 Wall-nut - Blocks zombie movement',
      '🍒 Cherry Bomb - Instant kill explosion'
    ],
    options: 'Options...',
    exitGame: 'Exit Game',
    languageTitle: 'Select Language',
    chinese: '中文',
    english: 'English',
    close: 'Close'
  }
};

interface LoginScreenProps {
  onStartGame: (isMultiplayer?: boolean) => void;
  onExitGame: () => void;
}

function MinecraftButton({
  children,
  small = false,
  onClick
}: {
  children: React.ReactNode;
  small?: boolean;
  onClick?: () => void;
}) {
  return (
    <button
      onClick={onClick}
      className={`
        w-full
        ${small ? 'py-2 text-sm' : 'py-3 text-lg'}
        font-medium
        text-white
        bg-gray-600/80
        hover:bg-gray-500/90
        active:bg-gray-700/90
        border-2 border-gray-900
        shadow-minecraft
        hover:shadow-minecraft-hover
        active:shadow-minecraft-active
        transition-all duration-75
        select-none
        relative
      `}
      style={{
        textShadow: '2px 2px 0px rgba(0,0,0,0.5)',
        fontFamily: 'Arial, sans-serif'
      }}
    >
      <span className="relative z-10">{children}</span>
      <div
        className="absolute inset-0 border-t-2 border-l-2 border-white/20 pointer-events-none"
      />
      <div
        className="absolute inset-0 border-b-2 border-r-2 border-black/40 pointer-events-none"
      />
    </button>
  );
}

function MinecraftButtonIcon({
  children,
  onClick
}: {
  children: React.ReactNode;
  onClick?: () => void;
}) {
  return (
    <button
      onClick={onClick}
      className="
        w-12 h-12
        flex items-center justify-center
        bg-gray-600/80
        hover:bg-gray-500/90
        active:bg-gray-700/90
        border-2 border-gray-900
        shadow-minecraft
        hover:shadow-minecraft-hover
        active:shadow-minecraft-active
        transition-all duration-75
        select-none
        relative
      "
    >
      <span className="relative z-10">{children}</span>
      <div
        className="absolute inset-0 border-t-2 border-l-2 border-white/20 pointer-events-none"
      />
      <div
        className="absolute inset-0 border-b-2 border-r-2 border-black/40 pointer-events-none"
      />
    </button>
  );
}

export function LoginScreen({ onStartGame, onExitGame }: LoginScreenProps) {
  const [showHowToPlay, setShowHowToPlay] = useState(false);
  const [showLanguage, setShowLanguage] = useState(false);
  const [currentLang, setCurrentLang] = useState<'zh' | 'en'>(() => {
    return (localStorage.getItem('pvz_language') as 'zh' | 'en') || 'zh';
  });

  const lang = LANGUAGES[currentLang];

  const handleLanguageChange = (newLang: 'zh' | 'en') => {
    setCurrentLang(newLang);
    localStorage.setItem('pvz_language', newLang);
    setShowLanguage(false);
  };

  return (
    <div className="min-h-screen relative overflow-hidden">
      {/* Background gradient */}
      <div
        className="absolute inset-0"
        style={{
          backgroundImage: `
            linear-gradient(135deg, #0a0e27 0%, #1a1f3a 25%, #0d1428 50%, #1a1f3a 75%, #0a0e27 100%),
            repeating-linear-gradient(0deg, transparent, transparent 20px, rgba(255,255,255,0.03) 20px, rgba(255,255,255,0.03) 40px),
            repeating-linear-gradient(90deg, transparent, transparent 20px, rgba(255,255,255,0.03) 20px, rgba(255,255,255,0.03) 40px)
          `,
          backgroundColor: '#0f1423'
        }}
      />
      {/* Purple glow effects */}
      <div
        className="absolute inset-0"
        style={{
          backgroundImage: `
            radial-gradient(circle at 20% 50%, rgba(139, 0, 139, 0.1) 0%, transparent 50%),
            radial-gradient(circle at 80% 80%, rgba(75, 0, 130, 0.1) 0%, transparent 50%)
          `,
          pointerEvents: 'none'
        }}
      />

      {/* Main content */}
      <div className="relative z-10 min-h-screen flex flex-col items-center justify-between py-8 px-4">
        <div className="flex-1 flex flex-col items-center justify-center w-full max-w-3xl">
          {/* Title - PVZ Style */}
          <div className="mb-8 text-center">
            <div className="flex items-center justify-center gap-2 mb-4">
              {/* PLANTS */}
              <span
                className="text-6xl md:text-7xl font-black select-none tracking-tight"
                style={{
                  color: '#7EC850',
                  textShadow: `
                    4px 4px 0px #2D5016,
                    5px 5px 0px #1A3009,
                    -2px -2px 0px #1A3009,
                    2px -2px 0px #1A3009,
                    -2px 2px 0px #1A3009,
                    0 0 10px rgba(126, 200, 80, 0.5)
                  `,
                  fontFamily: 'Impact, Arial Black, sans-serif',
                  WebkitTextStroke: '3px #2D5016',
                  paintOrder: 'stroke fill'
                }}
              >
                PLANTS
              </span>

              {/* VS. */}
              <span
                className="text-3xl md:text-4xl font-black select-none mx-1"
                style={{
                  color: '#D4A574',
                  textShadow: `
                    2px 2px 0px #5C4033,
                    3px 3px 0px #3D2817,
                    -1px -1px 0px #3D2817,
                    1px -1px 0px #3D2817
                  `,
                  fontFamily: 'Impact, Arial Black, sans-serif',
                  WebkitTextStroke: '2px #5C4033',
                  paintOrder: 'stroke fill'
                }}
              >
                VS.
              </span>

              {/* ZOMBIES */}
              <span
                className="text-6xl md:text-7xl font-black select-none tracking-tight"
                style={{
                  color: '#B8B8B8',
                  textShadow: `
                    4px 4px 0px #4A4A4A,
                    5px 5px 0px #2D2D2D,
                    -2px -2px 0px #2D2D2D,
                    2px -2px 0px #2D2D2D,
                    -2px 2px 0px #2D2D2D,
                    0 0 8px rgba(100, 100, 100, 0.4)
                  `,
                  fontFamily: 'Impact, Arial Black, sans-serif',
                  WebkitTextStroke: '3px #4A4A4A',
                  paintOrder: 'stroke fill'
                }}
              >
                ZOMBIES
              </span>
            </div>

            {/* MAZE EDITION */}
            <p
              className="text-2xl font-bold tracking-widest"
              style={{
                color: '#90EE90',
                textShadow: `
                  2px 2px 0px #2D5016,
                  -1px -1px 0px #1A3009,
                  1px -1px 0px #1A3009
                `,
                fontFamily: 'Impact, Arial Black, sans-serif',
                letterSpacing: '0.2em'
              }}
            >
              MAZE EDITION
            </p>
          </div>

          {/* Main buttons */}
          <div className="w-full max-w-2xl space-y-4">
            <MinecraftButton onClick={() => onStartGame(false)}>{lang.singlePlayer}</MinecraftButton>
            <MinecraftButton onClick={() => onStartGame(true)}>{lang.multiPlayer}</MinecraftButton>
            <MinecraftButton onClick={() => setShowHowToPlay(true)}>{lang.howToPlay}</MinecraftButton>
          </div>

          {/* Bottom buttons row */}
          <div className="w-full max-w-2xl mt-8 grid grid-cols-[auto_1fr_1fr_auto] gap-4 items-center">
            <MinecraftButtonIcon onClick={() => setShowLanguage(true)}>
              <Globe className="w-6 h-6 text-gray-200" />
            </MinecraftButtonIcon>

            <MinecraftButton small>{lang.options}</MinecraftButton>
            <MinecraftButton small onClick={onExitGame}>{lang.exitGame}</MinecraftButton>

            <MinecraftButtonIcon>
              <User className="w-6 h-6 text-gray-200" />
            </MinecraftButtonIcon>
          </div>
        </div>

      </div>

      {/* How to Play Popup */}
      {showHowToPlay && (
        <div className="fixed inset-0 z-50 flex items-center justify-center">
          {/* Overlay */}
          <div
            className="absolute inset-0 bg-black/80"
            onClick={() => setShowHowToPlay(false)}
          />
          {/* Popup */}
          <div className="relative z-10 bg-gray-800 border-4 border-gray-600 p-6 max-w-lg max-h-[80vh] overflow-y-auto mx-4"
               style={{
                 boxShadow: 'inset 2px 2px 0px rgba(255,255,255,0.2), inset -2px -2px 0px rgba(0,0,0,0.4)'
               }}>
            <button
              onClick={() => setShowHowToPlay(false)}
              className="absolute top-2 right-2 text-gray-400 hover:text-white"
            >
              <X className="w-6 h-6" />
            </button>
            <h2 className="text-2xl font-bold text-white mb-4 text-center"
                style={{ textShadow: '2px 2px 0px rgba(0,0,0,0.5)' }}>
              {lang.howToPlayTitle}
            </h2>
            <div className="text-green-300 text-sm leading-relaxed whitespace-pre-line"
                 style={{ fontFamily: 'Arial, sans-serif' }}>
              {lang.howToPlayContent.join('\n')}
            </div>
            <div className="mt-6 flex justify-center">
              <MinecraftButton small onClick={() => setShowHowToPlay(false)}>
                {lang.close}
              </MinecraftButton>
            </div>
          </div>
        </div>
      )}

      {/* Language Popup */}
      {showLanguage && (
        <div className="fixed inset-0 z-50 flex items-center justify-center">
          {/* Overlay */}
          <div
            className="absolute inset-0 bg-black/80"
            onClick={() => setShowLanguage(false)}
          />
          {/* Popup */}
          <div className="relative z-10 bg-gray-800 border-4 border-gray-600 p-6 max-w-sm mx-4"
               style={{
                 boxShadow: 'inset 2px 2px 0px rgba(255,255,255,0.2), inset -2px -2px 0px rgba(0,0,0,0.4)'
               }}>
            <button
              onClick={() => setShowLanguage(false)}
              className="absolute top-2 right-2 text-gray-400 hover:text-white"
            >
              <X className="w-6 h-6" />
            </button>
            <h2 className="text-xl font-bold text-white mb-4 text-center"
                style={{ textShadow: '2px 2px 0px rgba(0,0,0,0.5)' }}>
              {lang.languageTitle}
            </h2>
            <div className="space-y-3">
              <button
                onClick={() => handleLanguageChange('zh')}
                className={`w-full py-3 text-lg font-medium border-2 transition-all ${
                  currentLang === 'zh'
                    ? 'bg-green-700 border-green-500 text-white'
                    : 'bg-gray-600/80 border-gray-900 text-white hover:bg-gray-500/90'
                }`}
                style={{ textShadow: '2px 2px 0px rgba(0,0,0,0.5)' }}
              >
                {lang.chinese} {currentLang === 'zh' && '✓'}
              </button>
              <button
                onClick={() => handleLanguageChange('en')}
                className={`w-full py-3 text-lg font-medium border-2 transition-all ${
                  currentLang === 'en'
                    ? 'bg-green-700 border-green-500 text-white'
                    : 'bg-gray-600/80 border-gray-900 text-white hover:bg-gray-500/90'
                }`}
                style={{ textShadow: '2px 2px 0px rgba(0,0,0,0.5)' }}
              >
                {lang.english} {currentLang === 'en' && '✓'}
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
