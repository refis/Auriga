npcspawn("prontera.gat",160,320,0,"�e�X�gNPC#1",84,"PronteraMan001")
npctouch("�e�X�gNPC#1",2,2,"PronteraMan001_Touch")

function PronteraMan001(cid,oid)
	local name = "[�e�X�gNPC]"
	mes(name)
	mes("LuaNPC�̓���e�X�g�����܂��B")
	next()
	local switch={}
	switch[1]=function()
		mes(name)
		mes("���l���͗���\�����܂��B")
		next()
		local i = input(1)
		mes("���Ȃ��̓��͂������l��".. i .."�ł�")
		next()
		if select("���������","��߂�") == 2 then
			switch[3]()
		end
		switch[2]()
		fin()
	end
	switch[2]=function()
		mes(name)
		mes("��������͗���\�����܂��B")
		next()
		local s = input(2)
		mes("���Ȃ��̓��͂����������".. s .."�ł�")
		next()
		if select("���l����","��߂�") == 2 then
			switch[3]()
		end
		switch[1]()
		fin()
	end
	switch[3]=function()
		mes(name)
		mes("�e�X�g�I�����܂��B")
		mes("�����ꂳ�܂ł����B")
		close()
		fin()
	end
	switch[select("���l����","������","��߂�")]()
	close()
end
function PronteraMan001_Touch(cid,oid)
	local name = "[�e�X�gNPC]"
	mes(name)
	mes("OnTouch����ł��B")
	close()
end
