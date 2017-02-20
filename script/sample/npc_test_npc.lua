npcspawn("prontera.gat",160,320,0,"テストNPC#1",84,"PronteraMan001")
npctouch("テストNPC#1",2,2,"PronteraMan001_Touch")

function PronteraMan001(cid,oid)
	local name = "[テストNPC]"
	mes(name)
	mes("LuaNPCの動作テストをします。")
	next()
	local switch={}
	switch[1]=function()
		mes(name)
		mes("数値入力欄を表示します。")
		next()
		local i = input(1)
		mes("あなたの入力した数値は".. i .."です")
		next()
		if select("文字列入力","やめる") == 2 then
			switch[3]()
		end
		switch[2]()
		fin()
	end
	switch[2]=function()
		mes(name)
		mes("文字列入力欄を表示します。")
		next()
		local s = input(2)
		mes("あなたの入力した文字列は".. s .."です")
		next()
		if select("数値入力","やめる") == 2 then
			switch[3]()
		end
		switch[1]()
		fin()
	end
	switch[3]=function()
		mes(name)
		mes("テスト終了します。")
		mes("おつかれさまでした。")
		close()
		fin()
	end
	switch[select("数値入力","文字列","やめる")]()
	close()
end
function PronteraMan001_Touch(cid,oid)
	local name = "[テストNPC]"
	mes(name)
	mes("OnTouch動作です。")
	close()
end
