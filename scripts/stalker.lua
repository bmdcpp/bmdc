--[[
** stalker.lua :: Version 1.0 English (translated from the original Hungarian version 1.3 This is why language of this script is so poor :-) Feel free to change my mistakes.)
** Written by: SZL 
** A client-side script, compatible with LUA 5.1., not tested on 5.2. platform
** Allows to create triggers with several options, such as:
	- It is possible to add to a trigger more than one 'To search' record, also more than one answer. Answers are randomised.
	- Three possible moods for answers: angry, normal and happy.  
	- Possible to add Optional search records to a trigger. In that case, script answers in case of finding in a mainchat message a 'To search' record match, only if finds one of these Optional search records too.
	- Possible to add Exceptions. In that case script gives no answer for a 'To search' record found as it finds match for one or more of these Exception records.
	- You can set a trigger 'on' for Speak-checking module. Speak checking means that script knows who made a post in the mainchat in the last minutes (time interval also possible to set). 
	  If a triggers's speak checking option is set 'on', script gives no answer for that user who posted in this interval. 
	  This is for avoid mistaken answers. Let's see an example. Let's create a very first trigger.
	  Typing in mainchat:
	  /stinsert 1 re
	  /stinsert 1 here again
	  /stanswer 1 Here again, <nick> :) 2
	  /stanswer 1 Hi <nick>, good to see you again! 2
	  /stlist
	- Results this:
	-------------------------------------------------
	1. *Trigger* 
	-------------------------------------------------
 	*To search* 
 		re (1 1)
 		here again (1 2)
	-------------------------------------------------
 	*Optional search* 
	-------------------------------------------------
 	*Exceptions* 
 	-------------------------------------------------
 	*Answers* 
 		1. Angry mood:
		2. Normal mood:
			Here again, <nick> :) (1 1 2)
			Hi <nick>, good to see you again! (1 2 2)
		3. Happy mood:
	-------------------------------------------------
 	*Speak checking* Off
	-------------------------------------------------
 	*Action* No action (0)
	-------------------------------------------------
	
	- And it will work like this:
	  		-<[OP]Jonathan> Re
	  		-<Yourself> Here again, Jonathan :)
	  		-<[VIP]Somebody Else> Re, Jonathan!
	  		-<Yourself> Here again, Somebody Else!
	 - This is wrong, Somebody Else was here already :)
	 - SO, you must type:
	 /stspoken 1 on
	 You can also set the time interval for speak checking:
	 /stinterval 15 
	 - In this case, script will no answer to Somebody Else, if he did a post in the last 15 minutes.
	Also:
	- You can personalize your automatic answers by adding alternative nicks to some users's normal nicks (only one for each nick), to your friends perhaps. Ex.: If you set an alternative nick for a user, like: [OP]Jonathan -> Johny, answers will use Hi, Johny! instead of Ki, Jonathan!.
	- Delayed answers (This means that script gives answers after a few (given) seconds, not so fast as usual. 
	  By this, users do not realise immediately that your post is created automatically, otherwise they feel uncomfortable or something like that, in some cases such I've experienced.)
	- For further help type /sthelp after running script.
]]

function HubName(hub)
	local hubname = string.gsub(tostring(hub:getUrl()),"%.","_")
	local hubname = string.gsub(hubname,":","__")
	return hubname
end

function HubIndex(hub)
local hn = HubName(hub)
for i = 1,#stalker do
	if stalker[i] == hn then return i end
end
end

function UserIndex(index, nick)
local ret = nil
for i = 1,#stalker.speakers[index] do
	if stalker.speakers[index][i] == nick then ret = i break end
end
return ret
end

function HasSpoken (nick, index)
local hasspoken = false
 for i = 1,#stalker.speakers[index] do
	if nick == stalker.speakers[index][i] then hasspoken = true break end
 end
return hasspoken
end

function UserIsPal(nick)
local ispal = nil
local palindex = nil
for i = 1,#stalker.pal do
	if nick == stalker.pal[i][1] then 
		ispal = stalker.pal[i][2]
		palindex = i
		break
	end
end
return ispal, palindex
end

function CheckFile(f,dat)
local f1 = io.open( f, "r" )
	if f1 then 
		f1:close()
	else
		f1 = io.open( f, "w" )
		f1:write(dat)
		f1:close()
	end
end

function loadTrigger()
	trigger.call = {}
	trigger.answer = {}
	trigger.answer[1] = {}
	trigger.answer[2] = {}
	trigger.answer[3] = {}
	trigger.plus = {}
	trigger.exception = {}
	trigger.action = {}
	trigger.interval = {}
	
	local f = io.open( triggerFile, "r" )
	local tx = {}
	local i = 1
	local t = ""
	local t1 = nil
	repeat
		t1 = t
		t = f:read()
		if t == "<endoftriggers>" then
		
		elseif t == "<endoftrig>" then
			trigger.call[i] = tx
			tx = {}
			trigger.answer[1][i] = {}
			trigger.answer[2][i] = {}
			trigger.answer[3][i] = {}
			trigger.plus[i] = {}
			trigger.exception[i] = {}
		elseif t == "<endofansw1>" then
			trigger.answer[1][i] = tx
			tx = {}
		elseif t == "<endofansw2>" then
			trigger.answer[2][i] = tx
			tx = {}
		elseif t == "<endofansw3>" then
			trigger.answer[3][i] = tx
			tx = {}
		elseif t == "<endofplus>" then
			trigger.plus[i] = tx
			tx = {}
		elseif t == "<endofexception>" then
			trigger.exception[i] = tx
			tx = {}
		elseif t == "<endofinterval>" then
			trigger.interval[i] = tonumber(tx[1])
			tx = {}
		elseif t == "<endofaction>" then
			trigger.action[i] = tonumber(tx[1])
			tx = {}
			i = i + 1
		else
			table.insert(tx,t)
		end
	until (t == "<endoftriggers>")
	f:close()
end


function saveTrigger()
	local f = io.open( triggerFile, "w" )
	for i = 1,#trigger.call do
		for j = 1,#trigger.call[i] do
			f:write(trigger.call[i][j].."\n")
		end			
			f:write("<endoftrig>".."\n")
		for j = 1,#trigger.answer[1][i] do
			f:write(trigger.answer[1][i][j].."\n")
		end			
			f:write("<endofansw1>".."\n")
		for j = 1,#trigger.answer[2][i] do
			f:write(trigger.answer[2][i][j].."\n")
		end			
			f:write("<endofansw2>".."\n")
		for j = 1,#trigger.answer[3][i] do
			f:write(trigger.answer[3][i][j].."\n")
		end			
			f:write("<endofansw3>".."\n")
		for j = 1,#trigger.plus[i] do
			f:write(trigger.plus[i][j].."\n")
		end			
			f:write("<endofplus>".."\n")
		for j = 1,#trigger.exception[i] do
			f:write(trigger.exception[i][j].."\n")
		end			
			f:write("<endofexception>".."\n")
			f:write(trigger.interval[i].."\n")
			f:write("<endofinterval>".."\n")
			f:write(trigger.action[i].."\n")
			f:write("<endofaction>".."\n")


	end
	f:write("<endoftriggers>".."\n")
	f:close()
		
end

function loadPal()
local f = io.open( stalkerpalFile, "r" )
local line = nil
local i = 1
	line = f:read()
while line do	
	table.insert(stalker.pal, {})
	stalker.pal[i][1] = line
	line = f:read()
	stalker.pal[i][2] = line
	line = f:read()
	i = i + 1
end
f:close()
end

function savePal()
local f = io.open( stalkerpalFile, "w" )
for i = 1,#stalker.pal do
f:write(stalker.pal[i][1].."\n")
f:write(stalker.pal[i][2].."\n")
end
f:close()
end

function ShortNick(text)
local _,_,pref,nick = string.find(text,"^([%[%{%(]%S+[%)%}%]])(%S+)")
	if pref then text = nick end
return text
end

function triggerFind(what, inwhat, index)
local exist = false
	
	if string.find(inwhat," "..what.." ") then 
		exist = true 
	else
		local list = {",","%.","%;","%?","%!","%:","%;"}
		for i = 1,#list do
			if string.find(inwhat," "..what..list[i]) then
			exist = true
			break
			end
		end
	end
	if exist then
  	if #trigger.exception[index] > 0 then
		for i = 1,#trigger.exception[index] do
		if string.find(inwhat,string.lower(trigger.exception[index][i])) then exist = false break end
		end
  	end
	end
	if exist then
  	if #trigger.plus[index] > 0 then
		exist = false
		for i = 1,#trigger.plus[index] do
		if string.find(inwhat,string.lower(trigger.plus[index][i])) then exist = true break end
		end
 	 end
	end
  	
return exist
end


function ExecuteCommands(hub, text)
		local _,_,command = string.find(text,"^(%S+)")
		local _,_,command1, p1, p2 = string.find(text,"^(%S+)%s+(%d+)%s+(.+)")
		local _,_,command2, p3 = string.find(text,"^(%S+)%s+(%S+)")
		local _,_,command4, p4, p5, p6 = string.find(text,"^(%S+)%s+(%d+)%s+(.+)%s+(%d+)")
		local _,_,command5, p7, p8 = string.find(text,"^(%S+)%s+(%S+)%s+(%S+)")

		if command == commandPrefix.."stanswer" then
			local p = tonumber(p4)
			local pp = tonumber(p6)
			if p and p <= #trigger.answer[1] then
					if pp and pp > 0 and pp < 4  then
						table.insert(trigger.answer[pp][p],p5)
						hub:addLine("stalker.lua :: Answer added.")
					else 
						hub:addLine("stalker.lua :: Wrong parameter! Usage: /stanswer <nr of trigger> <text> <mood> (1 = angry 2 = normal 3 = happy)")
					end
			else hub:addLine("stalker.lua :: Wrong parameter! Usage: /stanswer <nr of trigger> <text> <mood> (1 = angry 2 = normal 3 = happy)")
			end
		elseif command == commandPrefix.."stplus" then
			local p = tonumber(p1)
			if p and p <= #trigger.plus then
				if p2 then
				table.insert(trigger.plus[p],p2)
				hub:addLine("stalker.lua :: Added to optional search.")
				end
			else hub:addLine("stalker.lua :: Wrong parameter! Usage: /stplus <nr of trigger> <text>")
			end
		elseif command == commandPrefix.."stexception" then
			local p = tonumber(p1)
			if p and p <= #trigger.exception then
				if p2 then
				table.insert(trigger.exception[p],p2)
				hub:addLine("stalker.lua :: Added to exceptions.")
				end
			else hub:addLine("stalker.lua :: Wrong parameter! Usage: /stexception <nr of trigger> <text>")
			end

		elseif command == commandPrefix.."stspoken" then
			local p = tonumber(p1)
			local post = "stalker.lua :: /stspoken <nr of trigger> <on/off>."
			if p and p <= #trigger.call then

				if p2 then

					if p2 == "on" then
						trigger.interval[p] = 1
						hub:addLine("stalker.lua :: OK")
					elseif p2 == "off" then
						trigger.interval[p] = 0
						hub:addLine("stalker.lua :: OK")
					else
						hub:addLine(post)
					end
				else
					hub:addLine(post)
				end
			else
				hub:addLine(post)
			end
		elseif command == commandPrefix.."staction" then
			local p = tonumber(p1)
			local post = "stalker.lua :: /staction <nr of trigger> <action>. Actions: 0 - no action; 1 - kick."
			if p and p <= #trigger.call then
				if p2 then

					if p2 == "0" then
						trigger.action[p] = 0
						hub:addLine("stalker.lua :: OK")
					elseif p2 == "1" then
						trigger.action[p] = 1
						hub:addLine("stalker.lua :: OK")
					else
						hub:addLine(post)
					end
				else
					hub:addLine(post)
				end
			else
				hub:addLine(post)
			end
		elseif command == commandPrefix.."stinsert" then
			if p1 then 
				if tonumber(p1) == nil then 
					hub:addLine("stalker.lua :: Wrong parameter! Usage: /stinsert <nr of trigger> <text>") 
			 	else 
					p1 = tonumber(p1)
				end
			if trigger.call == nil then 
			  trigger.call[1] = {}
			  p1 = 1	
			elseif trigger.call[p1] == nil then
			  p1 = #trigger.call + 1
			  trigger.call[p1] = {}
			  trigger.interval[p1] = 0
			  trigger.action[p1] = 0
			end
			table.insert(trigger.call[p1],p2)
			if trigger.answer[1][p1] == nil then trigger.answer[1][p1] = {} end
			if trigger.answer[2][p1] == nil then trigger.answer[2][p1] = {} end
			if trigger.answer[3][p1] == nil then trigger.answer[3][p1] = {} end
			if trigger.plus[p1] == nil then trigger.plus[p1] = {} end
			if trigger.exception[p1] == nil then trigger.exception[p1] = {} end
			hub:addLine("stalker.lua :: Added to search-list.")
			else
			hub:addLine("stalker.lua :: Wrong parameter! Usage: /stinsert <nr of trigger> <text>")
			end
		elseif command == commandPrefix.."stinterval" then
			if p3 then
				if tonumber(p3) and tonumber(p3) > 0 then
					hub:addLine("stalker.lua :: Time interval set to "..tonumber(p3).." minutes.")
					interval = tonumber(p3)*60
				else
					hub:addLine("stalker.lua :: Time interval must be set in minutes.")	
				end
			else
					hub:addLine("stalker.lua :: Time interval: "..(interval/60).." minutes.")
			end
		elseif command == commandPrefix.."stdelay" then
			if p3 then
			local ph = tonumber(p3)
				if ph and ph > 1 then
					hub:addLine("stalker.lua :: Answer delay set to "..tonumber(p3).." seconds.")
					delay = ph
				else
					hub:addLine("stalker.lua :: Answer delay must be set in seconds (minimum value is 2).")	
				end
			else
					hub:addLine("stalker.lua :: Answer delay: "..delay.." seconds.")
			end
		elseif command == commandPrefix.."stsave" then
			saveTrigger()
			hub:addLine("stalker.lua :: All triggers saved OK.")
		elseif command == commandPrefix.."stload" then
			loadTrigger()
			hub:addLine("stalker.lua :: Triggers loaded OK.")
		elseif command == commandPrefix.."stlist" then
			if #trigger.call == 0 then hub:addLine("stalker.lua :: The list is empty!")
			elseif p3 then
			 if tonumber(p3) and tonumber(p3) > 0 and tonumber(p3) <= #trigger.call then
				local csik = "-------------------------------------------------------------------------------------------------------------------\n"
				local temp = ""
				local r = tonumber(p3)
				local st, st1, st2, st3 = " "," "," "," "
				local st4 , st5 = " ", " "
			  		for j = 1,#trigger.call[r] do
			  			st = st.."\t"..trigger.call[r][j].." ("..r.." "..j..")".."\n"
			  		end
			  		if trigger.answer == {} then
			  		else
						st1 = st1.."\t1. Angry mood:\n"
			  			for j = 1,#trigger.answer[1][r] do
			  				st1 = st1.."\t\t"..trigger.answer[1][r][j].." ("..r.." "..j.." ".."1)\n"
			  			end
						st1 = st1.."\t2. Normal mood:\n"
			  			for j = 1,#trigger.answer[2][r] do
			  				st1 = st1.."\t\t"..trigger.answer[2][r][j].." ("..r.." "..j.." ".."2)\n"
			  			end
						st1 = st1.."\t3. Happy mood:\n"
			  			for j = 1,#trigger.answer[3][r] do
			  				st1 = st1.."\t\t"..trigger.answer[3][r][j].." ("..r.." "..j.." ".."3)\n"
			  			end
			  		end
			  		for j = 1,#trigger.plus[r] do
			  			st2 = st2..trigger.plus[r][j].." ("..r.." "..j..")\n"
			  		end
			  		for j = 1,#trigger.exception[r] do
			  			st3 = st3..trigger.exception[r][j].." ("..r.." "..j..")\n"
			  		end
					if trigger.interval[r] == 1 then st4 = "On" else st4 ="Off" end
					if trigger.action[r] == 0 then st5 = "No action (0)" else st5 ="Kick (1)" end
			 		temp = csik..r..". *Trigger* \n"..csik.." *To search* \n"..st..csik.." *Optional search* \n"..st2..csik.." *Exceptions* \n"..st3..csik.." *Answers* \n"..st1..csik.." *Speak checking* "..st4.."\n"..csik.." *Action* "..st5.."\n"..csik

					hub:addLine("\n"..temp.."stalker.lua :: Note: For help, after each element you see the numbers which u must use \n in case of deleting that record. Ex: /staremove 1 3 2 deletes the 3rd answer of normal (2nd) mood from the 1th trigger.")
			 else hub:addLine("stalker.lua :: No trigger with this number!")
			 end
			else
				local temp = "stalker.lua :: List of triggers by their number:\n"
				local st = ""
				for i = 1,#trigger.call do
				st = ""
					for j = 1,#trigger.call[i] do
						st = st..trigger.call[i][j].."; "
					end
				temp = temp..i..". "..st.."\n"
				end
				hub:addLine(temp.."OK")
			
			end	
		elseif command == commandPrefix.."stremove" then
			if not p1 or tonumber(p1) == nil or not p2 or tonumber(p2) == nil then
				hub:addLine("stalker.lua :: Wrong parameters!")
			elseif tonumber(p1) < 0 or tonumber(p1) > #trigger.call then
				hub:addLine("stalker.lua :: No trigger with this number!")
			elseif tonumber(p2) < 0 or tonumber(p2) > #trigger.call[tonumber(p1)] then
				hub:addLine("stalker.lua :: No record with this number in the trigger nr "..tonumber(p1))
			else
				table.remove(trigger.call[tonumber(p1)],tonumber(p2))
				hub:addLine("stalker.lua :: Search record deleted.")
			end
		elseif command == commandPrefix.."staremove" then
			if not p4 or tonumber(p4) == nil or not p5 or tonumber(p5) == nil or not p6 or tonumber(p6) == nil then
				hub:addLine("stalker.lua :: Wrong parameters!")
			elseif tonumber(p4) < 0 or tonumber(p4) > #trigger.call then
				hub:addLine("stalker.lua :: No trigger with this number!")
			elseif tonumber(p5) < 0 or tonumber(p5) > #trigger.answer[tonumber(p6)][tonumber(p1)] then
				hub:addLine("stalker.lua :: No record with this number in trigger nr "..tonumber(p4))
			else
				table.remove(trigger.answer[tonumber(p6)][tonumber(p4)],tonumber(p5))
				hub:addLine("stalker.lua :: Answer deleted.")
			end
		elseif command == commandPrefix.."stpremove" then
			if not p1 or tonumber(p1) == nil or not p2 or tonumber(p2) == nil then
				hub:addLine("stalker.lua :: Wrong parameters!")
			elseif tonumber(p1) < 0 or tonumber(p1) > #trigger.plus then
				hub:addLine("stalker.lua :: No trigger with this number!")
			elseif tonumber(p2) < 0 or tonumber(p2) > #trigger.plus[tonumber(p1)] then
				hub:addLine("stalker.lua :: No record with this number in trigger nr "..tonumber(p1))
			else
				table.remove(trigger.plus[tonumber(p1)],tonumber(p2))
				hub:addLine("stalker.lua :: Optional search record deleted.")
			end
		elseif command == commandPrefix.."steremove" then
			if not p1 or tonumber(p1) == nil or not p2 or tonumber(p2) == nil then
				hub:addLine("stalker.lua :: Wrong parameters!")
			elseif tonumber(p1) < 0 or tonumber(p1) > #trigger.exception then
				hub:addLine("stalker.lua :: No trigger with this number!")
			elseif tonumber(p2) < 0 or tonumber(p2) > #trigger.exception[tonumber(p1)] then
				hub:addLine("stalker.lua :: No record with this number in trigger nr "..tonumber(p1))
			else
				table.remove(trigger.exception[tonumber(p1)],tonumber(p2))
				hub:addLine("stalker.lua :: Exception record deleted.")
			end

		elseif command == commandPrefix.."stkill" then
			if not p3 or tonumber(p3) == nil then
				hub:addLine("stalker.lua :: Usage: /stkill <nr of trigger>")
			elseif tonumber(p3) < 0 or tonumber(p3) > #trigger.call then
				hub:addLine("stalker.lua :: No trigger with this number!")
			else
				p3 = tonumber(p3)
				table.remove(trigger.call,p3)
				table.remove(trigger.answer[1],p3)
				table.remove(trigger.answer[2],p3)
				table.remove(trigger.answer[3],p3)
				table.remove(trigger.plus,p3)
				table.remove(trigger.exception,p3)
				table.remove(trigger.interval,p3)
				table.remove(trigger.action,p3)
				hub:addLine("stalker.lua :: "..p3..". trigger deleted OK.")
			end
		elseif command == commandPrefix.."stsetmood" then
			if p3 and tonumber(p3) < 4 and tonumber(p3) > 0 then
				MOOD = tonumber(p3)
				hub:addLine("stalker.lua :: Mood has been set to: "..MOOD)
			else
				hub:addLine("Usage: /setmood <mood> (where mood is: 1 = angry 2 = normal 3 = happy)")
			end
		elseif command == commandPrefix.."stgetmood" then
			local mood = "normal"
				if MOOD == 1 then mood = "angry" elseif MOOD == 3 then mood = "happy" end
				hub:addLine("stalker.lua :: My mood is set to: "..mood)
		elseif command == commandPrefix.."stenable" then
			hub:addLine("stalker.lua :: Enabled on this hub.")
			stalker.run[HubIndex(hub)] = true	
		elseif command == commandPrefix.."stdisable" then
			hub:addLine("stalker.lua :: Disabled on this hub.")
			stalker.run[HubIndex(hub)] = false
		elseif command == commandPrefix.."stallenable" then
			for i = 1,#stalker.run do 
				stalker.run[i] = true
			end
			hub:addLine("stalker.lua :: Enabled on all hubs.")
		elseif command == commandPrefix.."stalldisable" then
			for i = 1,#stalker.run do 
				stalker.run[i] = false
			end
			hub:addLine("stalker.lua :: Disabled on all hubs.")

		elseif command == commandPrefix.."ststatus" then
			if stalker.run[HubIndex(hub)] then
				hub:addLine("stalker.lua :: Enabled on this hub.")
			else
				hub:addLine("stalker.lua :: Disabled on this hub.")
			end
		elseif command == commandPrefix.."sthublist" then
			local st = "stalker.lua ::\n"
			local stemp = " (not Operator)"
			for j = 1,#stalker do
			if stalker.isop[j] then stemp = " (Operator)" else stemp = " (not Operator)" end
				if stalker.run[j] then
					st = st..stalker[j]..": Enabled"..stemp.."\n"
				else
					st = st..stalker[j]..": Disabled"..stemp.."\n"
				end
			end
			hub:addLine(st.."OK")
		elseif command == commandPrefix.."stpal" then	
			if p7 and p8 then
				local isp, ind = UserIsPal(p7)
				if isp then
					stalker.pal[ind][2] = p8
					hub:addLine("stalker.lua :: OK "..p7.." Alternative nick changed: "..isp.." --> "..p8)
				else
					table.insert(stalker.pal,{})
					local tempind = #stalker.pal
					stalker.pal[tempind][1] = p7 
					stalker.pal[tempind][2] = p8
					hub:addLine("stalker.lua :: OK")
				end
				savePal()
			else
				hub:addLine("stalker.lua :: /stpal <nick> <alternative nick>")
			end
		elseif command == commandPrefix.."stlistpal" then
			hub:addLine("stalker.lua :: List of alternative nicks:")
			for i = 1,#stalker.pal do
				hub:addLine(stalker.pal[i][1].." -> "..stalker.pal[i][2])
			end
			hub:addLine("stalker.lua :: OK")
		elseif command == commandPrefix.."stremovepal" then
			if p3 then
				local tn, ti = UserIsPal(p3)
				if tn then
					table.remove(stalker.pal,ti)
					hub:addLine("stalker.lua :: OK")
					savePal()
				else
					hub:addLine("stalker.lua :: No such a nick on the list!")
				end
			else
				hub:addLine("stalker.lua :: /stremovepal <nick>")
			end
		elseif command == commandPrefix.."help" then	
			hub:addLine("(stalker.lua) /sthelp") 
			return nil
		elseif command == commandPrefix.."sthelp" then	
				hub:addLine("stalker.lua :: */sthelp* \r\n"..
					"*/stenable*   -Enables script on hub where typed.\r\n"..
					"*/stdisable*   -Disables script on hub where typed.\r\n"..
					"*/stallenable*   -Enables script on all hubs.\r\n"..
					"*/stalldisable*   -Disables script on all hubs (default).\r\n"..
					"*/sthublist*   -List of enabled and disabled hubs.\r\n"..
					"*/ststatus*   -Shows if hub where typed is enabled or disabled.\r\n"..
					"*/stlist*   -List of triggers by their numbers, with only 'To search' - records. \r\n"..
					"*/stlist* <nr of trigger>   -Shows trigger with number given. \r\n"..
					"*/stinsert* <nr of trigger> <text>   -New 'To search' - record for trigger with nr. It creates a new trugger in case nr is bigger than the biggest nr on the list.\r\n"..
					"*/stanswer* <nr of trigger> <text> <mood>   -New answer to trigger with <nr>. Mood must be 1, 2, or 3, such as 'angry', 'normal' or 'happy'. Any '<nick>' characters will be changed to short nick of user. Ex.: Hy, <nick>, happy to see you!\r\n"..
					"*/stplus* <nr of trigger> <text>   -Adds optional search for trigger with nr given.\r\n"..
					"*/stexception* <nr of trigger> <text>   -Adds an exception for trigger with nr given.\r\n"..
					"*/stspoken* <nr of trigger> <on/off>   -Speak checking for a trigger. If set to 'on', script gives no answer if user posted a message in mainchat in the last few minutes (minutes to set with /stsetinterval <minutes>)\r\n"..
					"*/staction* <nr of trigger> <0/1>   -Setting action of a trigger. 0 - No action; 1 - Kick; Note: at an answer-record of such a trigger you can use for example verlihub's parameters: /stanswer 1 You have been kicked._ban_1d 3\r\n"..
					"*/stremove* <nr of trigger> <nr of record>   -Deletes a trigger's 'To search' record.\r\n"..
					"*/staremove* <nr of trigger> <nr of answer record> <mood>   - Deletes an answer with parameters given.\r\n"..
					"*/stpremove* <nr of trigger> <nr of record>   -Deletes an optional search record.\r\n"..
					"*/steremove* <nr of trigger> <nr of record>   -Deletes an excetion.\r\n"..
					"*/stkill* <nr of trigger>    -Deletes a whole trigger.\r\n"..
					"*/stpal* <nick> <nick1>    -Sets alternative nick. Ex.: /stpal [HUN]Jonathan Johny - After this, any answer will be posted with nick1 given in case it contains one or more '<nick>'-s.\r\n"..
					"*/stremovepal* <nick>    -Deletes alternative nick for normal nick given.\r\n"..
					"*/stlistpal*    -List of alternative nick.\r\n"..
					"*/stinterval*  -Shows time interval for speak checking.\r\n"..
					"*/stinterval* <minutes>  -Sets the time interval.\r\n"..
					"*/stdelay*  -Shows the actual seconds of answer delay.\r\n"..
					"*/stdelay* <seconds>  -Sets the answer delay.\r\n"..
					"*/stsave*   -Saves triggers in a file. Use it frequently after modifications, because script does not save them automatically.\r\n"..
					"*/stload*   -Loads triggers. Any unsaved changes will be forgotten.\r\n"..
					"*/stsetmood* <mood>   -Setting up your mood (1, 2, 3, as 'angry', 'normal' or 'happyt'.)\r\n"..
	 				"*/stgetmood*   -Shows your actual mood. Mood is the same on all hubs connected.")
		end

	
end


-- Initializing
stalker = {}
stalker.run = {}
stalker.speakers = {}
stalker.time = {}
stalker.pal = {}
stalker.isop = {}

delay = 5
delayed = false
stalker.delayed = {}

interval = 20*60
last = os.time()+interval

trigger = {}
triggerFile = DC():GetConfigPath() .. "/scripts/stalker_triggers.txt"
stalkerpalFile = DC():GetConfigPath() .. "/scripts/stalker_nicklist.txt"

CheckFile(triggerFile,"<endoftriggers>")
CheckFile(stalkerpalFile,"")

loadTrigger()
loadPal()
commandPrefix = "/"

MOOD = 2
-- Checking hubs already connected
for k,hub in pairs(dcpp:getHubs()) do
	hub:addLine("stalker.lua :: Connected - For enable on this hub type: /stenable")
	table.insert(stalker,HubName(hub))
	table.insert(stalker.run,false)
	if hub:isOp(hub:getOwnNick()) then
		table.insert(stalker.isop,true)
	else
		table.insert(stalker.isop,false)
	end
	table.insert(stalker.speakers, {})
	table.insert(stalker.time,{})
end

RunTimer(1)

-- Listeners
dcpp:setListener( "chat", "focset",
	function( hub, user, text )
	local hi = HubIndex(hub)
	local ni = user:getNick()
	
	if #stalker.speakers[hi] > 25 then table.remove(stalker.speakers[hi],1) table.remove(stalker.time[hi],1) end	

	if stalker.run[hi] then
		text = " "..string.lower( text ).." "
		if trigger.call == {} or trigger.answer == {} then
		else
		local temp = {}
       		  for i = 1,#trigger.call do
			if trigger.interval[i] == 1 and HasSpoken(user:getNick(),HubIndex(hub)) then
			else
				for j = 1,#trigger.call[i] do
 			  		if triggerFind(string.lower(trigger.call[i][j]),text, i) then
			        		if #trigger.answer[MOOD][i] == 0 then
			  			else
							local st = trigger.answer[MOOD][i][math.random(#trigger.answer[MOOD][i])]
							if st then
								local name = nil
								local tn, ti = UserIsPal(ni)
								if tn then 
									name = tn
								else
									name = ShortNick(ni)
								end 
								st = string.gsub(st,"%b<>",name) 
							end
							if trigger.action[i] == 1 then
								if stalker.isop[hi] then
									hub:sendChat(st)
									hub:sendChat("!kick "..user:getNick().." "..st)
									break
								end
							else
								table.insert(temp, st)
							end							
						end
			  		end
				end
			end
		  end
		if #temp > 0 then
			table.insert(stalker.delayed,{})
			local t = #stalker.delayed
			stalker.delayed[t][1] = os.time() + delay
			stalker.delayed[t][2] = temp[math.random(#temp)]
			stalker.delayed[t][3] = HubName(hub)
			delayed = true
		end
		end
	end

	if HasSpoken(ni,hi) then 
		stalker.time[hi][UserIndex(hi, ni)] = os.time()
	else 
		table.insert(stalker.speakers[hi], ni) 
		table.insert(stalker.time[hi], os.time()) 
	end

	return nil
	end																			
)




dcpp:setListener( "ownChatOut", "commands",
	function( hub, text )
		text = FromUtf8(text) 
		if string.find(text,"^"..commandPrefix..".+") then
			ExecuteCommands(hub, text)
			return 1
		end
	end
)

dcpp:setListener( "connected", "in",
	function( hub )
		table.insert(stalker,HubName(hub))
		table.insert(stalker.run,false)

		table.insert(stalker.isop,false)

		table.insert(stalker.speakers,{})
		table.insert(stalker.time,{})
		hub:addLine("stalker.lua :: Connected OK - For enable on this hub type: /stenable")
		return nil
	end
)


dcpp:setListener( "disconnected", "out",
	function( hub )
		local hi = HubIndex(hub)
		table.remove(stalker,hi)
		table.remove(stalker.run,hi)
		table.remove(stalker.speakers,hi)
		table.remove(stalker.time,hi)
		table.remove(stalker.isop,hi)
		hub:addLine("stalker.lua :: Disconnected OK.")
		return nil
	end
)


dcpp:setListener( "timer", "myTimer",
	function()
if delayed then
	if os.time() > stalker.delayed[1][1] then
		for k,hub in pairs(dcpp:getHubs()) do
			if HubName(hub) == stalker.delayed[1][3] then 
				hub:sendChat(stalker.delayed[1][2]) 
			end
		end
	table.remove(stalker.delayed,1)
	if #stalker.delayed == 0 then delayed = false end
	end
end
		if os.time() > last then
			last = os.time() + interval
			local stemp, ttemp = {},{}
			for i = 1,#stalker do
			stemp, ttemp = {},{}
				for j = 1, #stalker.speakers[i] do					
					if os.time() > stalker.time[i][j] + interval then
					else
						table.insert(stemp,stalker.speakers[i][j])
						table.insert(ttemp,stalker.time[i][j])
					end
				end
			stalker.speakers[i] = stemp
			stalker.time[i] = ttemp
			end
		end
	end																			
)

dcpp:setListener( "userMyInfo", "myinfo",
	function( hub, user, myinfo )
	if user:getNick() == hub:getOwnNick() then
		if user:isOp() then
			stalker.isop[HubIndex(hub)] = true
		else
			stalker.isop[HubIndex(hub)] = false
		end
	end
	end
)


DC():PrintDebug( "** Loaded stalker.lua version 1.0 English**" )
