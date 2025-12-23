import { useState } from 'react';
import { LoginScreen } from './components/LoginScreen';
import { GameContainer } from './components/GameContainer';

type Screen = 'login' | 'game';

function App() {
  const [currentScreen, setCurrentScreen] = useState<Screen>('login');

  const handleStartGame = () => {
    setCurrentScreen('game');
  };

  const handleBackToMenu = () => {
    setCurrentScreen('login');
  };

  return (
    <>
      {currentScreen === 'login' && (
        <LoginScreen onStartGame={handleStartGame} />
      )}
      {currentScreen === 'game' && (
        <GameContainer onBack={handleBackToMenu} />
      )}
    </>
  );
}

export default App;
