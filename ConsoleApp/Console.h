#pragma once

struct StdBuffers;

class Console
{
private:
	std::unique_ptr<StdBuffers> m_StdBuffers;

public:
	Console();
	~Console();
};

