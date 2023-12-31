# Game Input Library

A small library for game input built on top of SDL. It attempts to have flexibility while always doing what players would reasonably expect, such as plugging in a game controller and have it start working. Game inputs can be defined and allow user configuration. The input library introduces the concept of different input "contexts" which can contain different inputs. Contexts can be layered so that for instance, customized player inputs can translate to a user-interface naviation context. Finally, the library provides a way to poll for input events, while allowing notification on the defined inputs as well as raw SDL events.

## Features

   * Definition of player input and other input contexts
   * Contexts can feed into other context layers via mappings
   * Contexts can be interpreted in preferred order
   * Default input configurations
   * Configuration for gamecontroller defaults
   * User-customization of player (or other context) inputs
   * Gamecontrollers are associated with players for multi-player setups
