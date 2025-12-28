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
    howToPlayTitle: '🎮 游戏玩法介绍',
    howToPlayContent: [
      '═══════════════════════════════════════',
      '',
      '🎯 【游戏目标】',
      '',
      '🧟 僵尸玩家：',
      '   在迷宫中找到出口并成功逃离！',
      '   躲避戴夫的追击和植物的攻击。',
      '',
      '👨‍🌾 戴夫玩家：',
      '   种植各种植物来阻止僵尸逃跑！',
      '   在僵尸到达出口前将其消灭。',
      '',
      '═══════════════════════════════════════',
      '',
      '🎮 【操作按键】',
      '',
      '📍 单人模式（控制僵尸）：',
      '   • WASD 或 方向键 - 控制移动',
      '   • Ctrl - 使用撑杆跳跃障碍物',
      '   • Tab 或 Enter - 打开/关闭小地图',
      '',
      '📍 多人模式（分屏对战）：',
      '   【戴夫 - 左半屏】',
      '   • WASD - 控制戴夫移动',
      '   • Q - 打开种植菜单',
      '   • Tab - 打开小地图',
      '',
      '   【僵尸 - 右半屏】',
      '   • 方向键 - 控制僵尸移动',
      '   • Ctrl - 撑杆跳跃',
      '   • Enter - 打开小地图',
      '',
      '═══════════════════════════════════════',
      '',
      '🎒 【道具系统】',
      '',
      '   🪣 铁桶',
      '      增加200点护甲值，可抵御植物攻击',
      '',
      '   🏃 撑杆跳套装',
      '      装备后移动速度提升，可跳过一格障碍物',
      '',
      '   ❤️ 生命药水',
      '      立即恢复40%的最大生命值',
      '',
      '   ⚡ 速度药水',
      '      短时间内移动速度提升50%',
      '',
      '═══════════════════════════════════════',
      '',
      '🌱 【植物介绍】',
      '',
      '   🌱 豌豆射手 (消耗100阳光)',
      '      向前方发射豌豆，对僵尸造成伤害',
      '',
      '   🌱 双发射手 (消耗200阳光)',
      '      一次发射两颗豌豆，伤害更高',
      '',
      '   🥜 坚果墙 (消耗50阳光)',
      '      高生命值，可有效阻挡僵尸前进',
      '',
      '   🍒 樱桃炸弹 (消耗150阳光)',
      '      范围爆炸，对范围内僵尸造成巨大伤害',
      '',
      '═══════════════════════════════════════',
      '',
      '💡 【游戏提示】',
      '',
      '   • 僵尸在攻击时可以按方向键取消攻击',
      '   • 小地图可以帮助你了解整个迷宫布局',
      '   • 合理利用道具可以大幅提升生存能力',
      ''
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
    howToPlayTitle: '🎮 How to Play',
    howToPlayContent: [
      '═══════════════════════════════════════',
      '',
      '🎯 【GAME OBJECTIVE】',
      '',
      '🧟 Zombie Player:',
      '   Navigate through the maze and escape!',
      '   Avoid Dave and plant attacks.',
      '',
      '👨‍🌾 Dave Player:',
      '   Plant defenses to stop the zombie!',
      '   Defeat the zombie before it escapes.',
      '',
      '═══════════════════════════════════════',
      '',
      '🎮 【CONTROLS】',
      '',
      '📍 Single Player (Control Zombie):',
      '   • WASD or Arrow Keys - Move',
      '   • Ctrl - Pole vault jump over obstacles',
      '   • Tab or Enter - Toggle minimap',
      '',
      '📍 Multiplayer (Split Screen):',
      '   【Dave - Left Screen】',
      '   • WASD - Move Dave',
      '   • Q - Open plant menu',
      '   • Tab - Toggle minimap',
      '',
      '   【Zombie - Right Screen】',
      '   • Arrow Keys - Move zombie',
      '   • Ctrl - Pole vault jump',
      '   • Enter - Toggle minimap',
      '',
      '═══════════════════════════════════════',
      '',
      '🎒 【ITEMS】',
      '',
      '   🪣 Bucket',
      '      Adds 200 armor points for protection',
      '',
      '   🏃 Pole Vault Kit',
      '      Faster movement, can jump one obstacle',
      '',
      '   ❤️ Health Potion',
      '      Instantly restore 40% max health',
      '',
      '   ⚡ Speed Potion',
      '      50% speed boost for a short duration',
      '',
      '═══════════════════════════════════════',
      '',
      '🌱 【PLANTS】',
      '',
      '   🌱 Peashooter (100 Sun)',
      '      Shoots peas at zombies',
      '',
      '   🌱 Repeater (200 Sun)',
      '      Fires two peas at once',
      '',
      '   🥜 Wall-nut (50 Sun)',
      '      High HP, blocks zombie movement',
      '',
      '   🍒 Cherry Bomb (150 Sun)',
      '      Area explosion, massive damage',
      '',
      '═══════════════════════════════════════',
      '',
      '💡 【TIPS】',
      '',
      '   • Zombies can cancel attacks by pressing direction keys',
      '   • Use minimap to understand the maze layout',
      '   • Items can greatly improve survival chances',
      ''
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
        <div className="fixed inset-0 z-50 flex items-center justify-center p-4">
          {/* Overlay */}
          <div
            className="absolute inset-0 bg-black/85"
            onClick={() => setShowHowToPlay(false)}
          />
          {/* Popup - 70% of screen */}
          <div className="relative z-10 bg-gray-800 border-4 border-green-700 p-8 overflow-y-auto"
               style={{
                 width: '70vw',
                 height: '85vh',
                 maxWidth: '900px',
                 boxShadow: 'inset 3px 3px 0px rgba(255,255,255,0.2), inset -3px -3px 0px rgba(0,0,0,0.4), 0 0 30px rgba(0,128,0,0.3)'
               }}>
            <button
              onClick={() => setShowHowToPlay(false)}
              className="absolute top-4 right-4 text-gray-400 hover:text-white transition-colors"
            >
              <X className="w-8 h-8" />
            </button>
            <h2 className="text-3xl font-bold text-green-400 mb-6 text-center"
                style={{ textShadow: '3px 3px 0px rgba(0,0,0,0.7)' }}>
              {lang.howToPlayTitle}
            </h2>
            <div className="text-green-300 text-lg leading-loose whitespace-pre-line"
                 style={{ fontFamily: 'Consolas, Monaco, monospace', fontSize: '18px' }}>
              {lang.howToPlayContent.join('\n')}
            </div>
            <div className="mt-8 flex justify-center">
              <MinecraftButton onClick={() => setShowHowToPlay(false)}>
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
                className={`w-full py-3 text-lg font-medium border-4 transition-all relative ${
                  currentLang === 'zh'
                    ? 'bg-green-700 border-green-400 text-white scale-105 shadow-lg'
                    : 'bg-gray-600/80 border-gray-700 text-gray-300 hover:bg-gray-500/90 hover:text-white'
                }`}
                style={{
                  textShadow: '2px 2px 0px rgba(0,0,0,0.5)',
                  boxShadow: currentLang === 'zh' ? '0 0 15px rgba(74, 222, 128, 0.5)' : 'none'
                }}
              >
                🇨🇳 {lang.chinese}
              </button>
              <button
                onClick={() => handleLanguageChange('en')}
                className={`w-full py-3 text-lg font-medium border-4 transition-all relative ${
                  currentLang === 'en'
                    ? 'bg-green-700 border-green-400 text-white scale-105 shadow-lg'
                    : 'bg-gray-600/80 border-gray-700 text-gray-300 hover:bg-gray-500/90 hover:text-white'
                }`}
                style={{
                  textShadow: '2px 2px 0px rgba(0,0,0,0.5)',
                  boxShadow: currentLang === 'en' ? '0 0 15px rgba(74, 222, 128, 0.5)' : 'none'
                }}
              >
                🇺🇸 {lang.english}
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
