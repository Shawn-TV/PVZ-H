import { useState } from 'react';
import { LoginScreen } from './components/LoginScreen';
import { GameContainer } from './components/GameContainer';

type Screen = 'login' | 'game';

function App() {
  const [currentScreen, setCurrentScreen] = useState<Screen>('login');
  const [isMultiplayer, setIsMultiplayer] = useState(false);

  const handleStartGame = (multiplayer: boolean = false) => {
    setIsMultiplayer(multiplayer);
    setCurrentScreen('game');
  };

  const handleBackToMenu = () => {
    setCurrentScreen('login');
  };

  const handleExitGame = () => {
    window.close();
  };

  return (
    <>
      {currentScreen === 'login' && (
        <LoginScreen onStartGame={handleStartGame} onExitGame={handleExitGame} />
      )}
      {currentScreen === 'game' && (
        <GameContainer onBack={handleBackToMenu} isMultiplayer={isMultiplayer} />
      )}
    </>
  );
}

export default App;
