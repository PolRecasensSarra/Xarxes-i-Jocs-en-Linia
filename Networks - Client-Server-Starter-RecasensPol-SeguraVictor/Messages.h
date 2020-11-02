#pragma once

// Add as many messages as you need depending on the
// functionalities that you decide to implement.

enum class ClientMessage
{
	Hello,
	Message,
	Command
};

enum class ServerMessage
{
	Welcome,
	NoWelcome,
	Join,
	Left,
	Chat,
	Command,
	Kick,
	Error,
	ChangeName,
	NewName,
	Whisper,
};

