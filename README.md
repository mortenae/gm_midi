# MIDI controller module for Garry's Mod 13

## Example

```lua
require("midi")

-- Key is the device ID, value is the device name
for k, v in ipairs(midi.GetAll()) do
	chat.AddText(string.format("%2d: %s", k, v))
end

MIDI = MIDI or midi.Open(1)
NOTES = NOTES or {}

MIDI:SetCallback(function(self, opcode, channel, data1, data2, time)
	if opcode == midi.NOTEOFF then
		for k, v in ipairs(NOTES) do
			if v == data1 then
				table.remove(NOTES, k)
				break
			end
		end
	elseif opcode == midi.NOTEON then
		table.insert(NOTES, data1)
	end
end)

MIDI:Start()

hook.Add("CreateMove", "midi", function(cmd)
	if #NOTES > 0 then
		local index = 1 + math.floor(SysTime() * 5) % #NOTES
		local note = NOTES[index]

		local angles = cmd:GetViewAngles()
		angles.pitch = (note - 52) / 12 * -89
		cmd:SetViewAngles(angles)
	
		cmd:SetButtons(bit.bor(cmd:GetButtons(), IN_ATTACK))
	end
end)
```
