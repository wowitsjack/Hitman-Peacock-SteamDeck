# HitmanRun 🎯🚀

HitmanRun is a streamlined launcher designed to simplify the process of running the **Peacock Server Emulator** on the **Valve Steam Deck** or **SteamOS**. The Peacock emulator mimics the functionality of IO Interactive's game servers, allowing players to self-host and manage their server experience. 

This tool removes the complexities associated with running the Peacock server on the Steam Deck, including handling Proton isolation and managing multiple windows. 

It's also intended to be used with (ie. I've tested it with) HITMAN III/WoA (Steam, latest)

---

## ✨ Features
- **Steam Deck Ready:** Fully compatible with the Steam Deck and SteamOS.
- **Simplified Launch Process:** Automatically handles the setup and execution of the Peacock server and game.
- **Customizable:** Works seamlessly with the **SteamGridDB Plugin** for artwork customization.

---

## 🛠️ Installation & Setup Guide

1. **Download Peacock**:
   - Go to [Peacock's official website](https://thepeacockproject.org) and download the latest release for Windows (Yes, I know you're on a Linux based Steam Deck).
   - Extract the Peacock release files into the HITMAN III install directory, alongside `Launcher.exe`.
   - Download the [latest Release](https://github.com/wowitsjack/Hitman-Peacock-SteamDeck/releases/download/1.0/Hitman-Peacock-SteamDeck-main.zip)
   - Extract these files also alongside `Launcher.exe`

2. **Add HitmanRun to Steam**:
   - Open Steam on your Steam Deck.
   - Go to `Games` > `Add a Non-Steam Game to My Library`.
   - Browse to `HitmanRun.exe` and add it.

3. **Launch HitmanRun**:
   - Set 'Compatibility Mode' to On in Properties for HitmanRun within Steam. (Right click -> Properties) (I suggest Proton 9+)
   - Launch `HitmanRun.exe` from your Steam Library.

5. **Switch Between Windows**:
   - Press the **Steam button** while in-game.
   - Scroll down to see the list of open windows.
   - Switch to the PeacockPatcher window.

6. **Connect to the Peacock Server**:
   - In the PeacockPatcher window, enter the following address:
     ```
     127.0.0.1:6969
     ```
     This should only be needed ONCE, it should save and auto-inject from here on with each launch.

7. **Enjoy the Game!**:
   - Switch back to the game window and start playing. 🎮

---

## 🎨 Bonus: Customize the Launcher with SteamGridDB Plugin

Take your customization to the next level by using the **SteamGridDB Plugin** to add the proper HITMAN artwork to your HitmanRun launcher.

### Install SteamGridDB Plugin:
1. **Install Decky Loader**:
   - Follow the instructions at the [Decky Loader GitHub page](https://github.com/SteamDeckHomebrew/decky-loader) to install it on your Steam Deck.
   
2. **Install the SteamGridDB Plugin**:
   - Open the **Quick Access Menu** on your Steam Deck.
   - Navigate to the `Plugins` section and browse the Plugin Store.
   - Search for **SteamGridDB** and install it.

### Customize HITMAN Artwork:
1. After installing the plugin, open it from the **Quick Access Menu**.
2. Select `HitmanRun.exe` from your library.
3. Browse and download high-quality the proper HITMAN artwork, including:
   - Grids
   - Heroes
   - Icons
4. Apply the artwork to make your launcher visually match the game.

---

## 🖋️ Rename the Game in Steam Deck GUI

To properly rename the launcher for consistency:
1. Open your Steam Library.
2. Find `HitmanRun.exe` in your list of games.
3. Right-click (or press the options button) and select `Properties`.
4. Under the **Shortcut** tab, rename it to `HITMAN III - Peacock`.

Now, your Steam library will display the proper name alongside custom artwork for a clean, polished experience.

---

## ⚠️ Notes
- Ensure all Peacock files are extracted into the same directory as `HitmanRun.exe`.
- The Peacock server uses `127.0.0.1:6969` by default. Ensure no other services are using this port.
- Customizing the artwork and renaming the launcher are optional steps but greatly enhance the experience.

---

## 💡 Where is the EXE source!?

- That .cmd file there, we just stamp it down into a flat executable exe.
- You're welcome to compile it yourself :D

## 💡 Why Use HitmanRun?
HitmanRun eliminates the challenges of setting up Peacock on the Steam Deck, providing a user-friendly solution that integrates seamlessly with Steam. By enabling custom artwork and streamlining server hosting, it lets you focus on what matters: enjoying the game.

---

Enjoy your game, Agent! 🕶️🔫
