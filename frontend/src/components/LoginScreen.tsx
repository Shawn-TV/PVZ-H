import { Globe, User } from 'lucide-react';

interface LoginScreenProps {
  onStartGame: () => void;
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
            <MinecraftButton onClick={onStartGame}>单人游戏</MinecraftButton>
            <MinecraftButton>多人游戏</MinecraftButton>
            <MinecraftButton>玩法介绍</MinecraftButton>
          </div>

          {/* Bottom buttons row */}
          <div className="w-full max-w-2xl mt-8 grid grid-cols-[auto_1fr_1fr_auto] gap-4 items-center">
            <MinecraftButtonIcon>
              <Globe className="w-6 h-6 text-gray-200" />
            </MinecraftButtonIcon>

            <MinecraftButton small>选项...</MinecraftButton>
            <MinecraftButton small onClick={onExitGame}>退出游戏</MinecraftButton>

            <MinecraftButtonIcon>
              <User className="w-6 h-6 text-gray-200" />
            </MinecraftButtonIcon>
          </div>
        </div>

      </div>
    </div>
  );
}
