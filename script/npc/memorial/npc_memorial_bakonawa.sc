//= Auriga Script ==============================================================
// Ragnarok Online Bakonawa Lake Script	by refis
//==============================================================================

//============================================================
// �_���W��������NPC
//------------------------------------------------------------
ma_scene01.gat,174,179,4	script	�^�z�[	541,{
	if(BaseLevel < 140) {
		mes "[�^�z�[]";
		mes "�����͂ƂĂ��댯�ȏꏊ�ł��B";
		mes "�X�ɖ߂��ĉ������I";
		next;
		mes "�]���̃N�G�X�g��i�s����ɂ�";
		mes "�@���x��������Ȃ��悤�ł��B";
		mes "�@^FF0000Base���x��140�ȏ�^000000�ɂȂ�����";
		mes "�@�ēx�b�������Ă��������]";
		close;
	}
	if(MALAYA_6QUE < 7 && MALAYA_7QUE < 15) {
		mes "[�^�z�[]";
		mes "�o�R�i���̂�����";
		mes "�������݂͂�Ȏ��ʂ�������܂���B";
		mes "�N���A�o�R�i����ގ����Ă����";
		mes "�����l������Ȃ����낤���c�c";
		close;
	}
	if(getonlinepartymember() < 1) {
		mes "[�^�z�[]";
		mes "���̐�͈�l�ł͊댯�ł��B";
		mes "�p�[�e�B�[���������ė��ĉ������B";
		close;
	}
	if(checkquest(12279) & 0x4) {
		mes "[�^�z�[]";
		mes "�o�R�i����ގ���������";
		mes "����Ȃ��̂��E���܂����B";
		mes "������������A���Ȃ�������";
		mes "���ɗ���������܂���ˁB";
		getitem 6499,5;
		if(checkre()) {
			getexp 500000,0;
			getexp 500000,0;
			getexp 500000,0;
			getexp 500000,0;
			getexp 500000,0;
			getexp 500000,0;
			getexp 0,300000;
		}
		else {
			getexp 5000000,0;
			getexp 5000000,0;
			getexp 0,5000000;
		}
		delquest 12279;
		close;
	}
	if(checkquest(12278)) {
		if(!(checkquest(12278) & 0x2)) {
			mes "[�^�z�[]";
			mes "�o�R�i���΂ɉ���邽�߂�";
			mes "���j���g���؂��Ă��܂��܂����B";
			mes "�p�ӂɎ��Ԃ�������̂ŁA";
			mes "�܂����x���Ă��������B";
			close;
		}
		mes "[�^�z�[]";
		mes "���j�̗p�ӂ��ł��܂����B";
		mes "����Ńo�R�i���΂�";
		mes "�s�����Ƃ��ł��܂��B";
		delquest 12278;
		close;
	}
	setquest 12279;
	if(getpartyleader(getcharid(1)) == strcharinfo(0)) {
		mes "[�^�z�[]";
		mes "�o�R�i���ގ���C���ꂽ�l�ł��ˁH";
		mes "��ǂ�����邽��";
		mes "�������菀�������Ă��������B";
		set '@str1$,"���j�������ĉ�����";
		set '@str2$,"�����牺��܂�";
	}
	else {
		mes "[�^�z�[]";
		mes "�o�R�i����ގ����邽�߂�";
		mes "���Ă��ꂽ��ł��ˁB";
		mes "���[�_�[�Ǝ��ŁA��ǂ�����邽�߂�";
		mes "���j�������܂��ˁB";
		set '@str2$,"�����牺��܂�";
	}
	next;
	switch(select('@str1$,'@str2$,"�L�����Z��")) {
	case 1:
		if(getpartyleader(getcharid(1)) != strcharinfo(0)) {
			mes "[�^�z�[]";
			mes "����H";
			mes "���̊Ԃɂ����[�_�[�ł�";
			mes "�Ȃ��Ȃ��Ă��܂��Ă��܂��ˁB";
			mes "���[�_�[��A��Ă��Ă��������B";
			close;
		}
		mdcreate "Bakonawa Lake";
		mes "[�^�z�[]";
		mes "����ł́A���j�������܂��B";
		close;
	case 2:
		switch(mdenter("Bakonawa Lake")) {
		case 0:	// �G���[�Ȃ�
			announce "[" +strcharinfo(1)+ "]�p�[�e�B�[��[" +strcharinfo(0)+ "]��[Bakonawa Lake]�ɓ��ꂵ�܂�",0x9,0x00FF99;
			setquest 12278;
			donpcevent getmdnpcname("BakonawaControl")+ "::OnStart";
			end;
		case 1:	// �p�[�e�B�[������
			mes "[�^�z�[]";
			mes "�N�̃p�[�e�B�[�ɐ�ǂ������";
			mes "���͏o�Ă��܂����B";
			close;
		case 2:	// �_���W�������쐬
			mes "[�^�z�[]";
			mes "�܂����j��̂Ɋ����Ă��܂���B";
			mes "���܂�Ȃ��ł��������B";
			close;
		default:	// ���̑��G���[
			close;
		}
	case 3:
		if(getpartyleader(getcharid(1)) == strcharinfo(0)) {
			mes "[�^�z�[]";
			mes "�������܂��I����Ă��Ȃ��̂ł����H";
			mes "�ł��邾�������������I��点�āA";
			mes "�o�R�i����ގ����Ă��������ˁI";
		}
		close;
	}
}

//============================================================
// �o�R�i���̐��ݏ�
//------------------------------------------------------------
1@ma_b.gat,0,0,0	script	BakonawaControl	-1,{
OnStart:
	if('flag > 0)
		end;
	set 'flag,1;
	hideonnpc getmdnpcname("�^�z�[#����");
	hideonnpc getmdnpcname("�^�z�[#���s");
	end;
}

1@ma_b.gat,63,52,4	script	�^�z�[#nf	541,{
	mes "[�^�z�[]";
	mes "���悢��o�R�i���Ƃ̐퓬�ł��B";
	mes "�ł��邱�ƂȂ玄�������������̂ł���";
	mes "�݂Ȃ���̂悤�ɋ����Ȃ��̂�";
	mes "��A�ɉB��Ă��܂��ˁB";
	next;
	mes "[�^�z�[]";
	mes "�o�R�i���́A���Ԃ��o��";
	mes "�΂̒�ɓ����Ă��܂��܂��B";
	mes "�܂��A�U�����ʂ��Ȃ���ԂɂȂ�";
	mes "�ꍇ������܂��B";
	mes "������A����T�|�[�g���܂��̂�";
	mes "�m�F���Ȃ������Ă��������B";
	next;
	if(getpartyleader(getcharid(1)) == strcharinfo(0)) {
		mes "[�^�z�[]";
		mes "�ł́A������o�R�i����";
		mes "�΂̏�Ɉ�������o���Č����܂��B";
		mes "�܂���10���ȓ��ɓ|���Ă��������B";
		next;
		if(select("�҂��āI�@�܂����I","�킩�����A�ł͎n�߂悤�I") == 1) {
			mes "[�^�z�[]";
			mes "���c�c��Ȃ�����Ȃ��ł����I";
			mes "���₤���Ăяo�����ł�����B";
			close;
		}
		mes "[�^�z�[]";
		mes "�ł́A����Ȗ���΂�";
		mes "��H���Ƃ��Ă݂܂��B";
		next;
		mes "[�^�z�[]";
		mes "�΂ւ̉e���͒Z���Ԃł����A";
		mes "�o�R�i���̓z�͕q���ɔ�������";
		mes "���ʂɔ�яo���Ă���ł��傤�B";
		donpcevent getmdnpcname("#�o�R�i��n1")+"::OnStart";
		hideonnpc getmdnpcname("�^�z�[#nf");
		close;
	}
	else {
		mes "[�^�z�[]";
		mes "�������ł�����A���[�_�[����";
		mes "�b��������悤�`���Ă��������B";
		close;
	}
}

1@ma_b.gat,36,111,4	script	#�o�R�i��n1	844,{
OnStart:
	announce "�^�z�[�F�o�R�i��������܂����I�@10���ȓ��ɓ|���Ă��������I",0x9,0x00ffff,0x190,15;
	monster getmdmapname("1@ma_b.gat"),78,83,"�o�R�i��",2320,1,getmdnpcname("#�o�R�i��n1")+"::OnKilled";
	initnpctimer;
	end;
OnKilled:
	donpcevent getmdnpcname("#�o�R�i������n1")+"::OnStart";
	stopnpctimer;
	hideonnpc getmdnpcname("#�o�R�i��n1");
	end;
OnTimer1000:	callsub OnAnnounce,"10��",15;
OnTimer60000:	callsub OnAnnounce,"9��",15;
OnTimer120000:	callsub OnAnnounce,"8��",15;
OnTimer180000:	callsub OnAnnounce,"7��",15;
OnTimer240000:	callsub OnAnnounce,"6��",15;
OnTimer300000:	callsub OnAnnounce,"5��",15;
OnTimer360000:	callsub OnAnnounce,"4��",15;
OnTimer420000:	callsub OnAnnounce,"3��",15;
OnTimer480000:	callsub OnAnnounce,"2��",15;
OnTimer540000:	callsub OnAnnounce,"1��",15;
OnTimer570000:	callsub OnAnnounce,"30�b",18;
OnTimer600000:
	announce "�o�R�i�����΂̒��ɓ����čs���܂����B",0x9,0xffff00,0x190,20;
	donpcevent getmdnpcname("�^�z�[#���s")+"::OnStart";
	killmonster getmdmapname("1@ma_b.gat"),getmdnpcname("#�o�R�i��n1")+"::OnKilled";
	stopnpctimer;
	hideonnpc getmdnpcname("#�o�R�i��n1");
	end;
OnAnnounce:
	announce "��������"+getarg(0),0x9,0xFF4400,0x190,getarg(1);
	donpcevent getmdnpcname("#�艺����n1")+"::OnStart";
	end;
}

1@ma_b.gat,78,81,0	script	#�艺����n1	139,4,5,{
OnStart:
	hideoffnpc getmdnpcname("#�艺����n1");
	initnpctimer;
	end;
OnTouch:
	stopnpctimer;
	hideonnpc getmdnpcname("#�艺����n1");
	end;
OnTimer5000:
	set '@map$,getmdmapname("1@ma_b.gat");
	setarray '@x,79,71,60,61,57,89,95,96,99;
	setarray '@y,71,72,80,90,99,73,82,90,99;
	for(set '@i,0; '@i<9; set '@i,'@i+1) {
		set '@rand,rand(1,10);
		if('@rand > 7)
			monster '@map$,'@x['@i],'@y['@i],"�o�R�i���̈ӎu",2337,1,getmdnpcname("#�艺����n1")+"::OnKilled";
		else if('@rand < 4)
			monster '@map$,'@x['@i],'@y['@i],"�o�R�i���̈ӎu",2343,1,getmdnpcname("#�艺����n1")+"::OnKilled";
	}
	hideonnpc getmdnpcname("#�艺����n1");
	end;
OnTimer50000:
	killmonster getmdmapname("1@ma_b.gat"),getmdnpcname("#�艺����n1")+"::OnKilled";
	stopnpctimer;
	end;
OnKilled:
	end;
}

1@ma_b.gat,1,5,4	script	#�o�R�i������n1	844,{
	end;
OnStart:
	initnpctimer;
	end;
OnTimer100:
	announce "�^�z�[�F�܂��ł��I�@�`���ɂ��ƁA�o�R�i���͌������ݍ������ƍĂюp��������͂��ł��I",0x9,0x00ffff,0x190,15;
	end;
OnTimer5000:
	announce "�^�z�[�F�z�����ɏW���ł��Ȃ��悤�ɁA��Ƒ��ۂ����邭�炢�@���Ďז����Ă��������I",0x9,0x00ffff,0x190,15;
	end;
OnTimer10000:
	announce "�^�z�[�F�z�����̏�Ɍ���Ă��������Ă��������B��Ƒ��ۂ�@�����ƂɏW�������ł��I",0x9,0x00ffff,0x190,15;
	end;
OnTimer15000:
	announce "�ڕW�]�΂̍��E�ɂ����Ƒ��ۂ�4�A����܂ōU������]",0x9,0xff3300,0x190,15;
	donpcevent getmdnpcname("#�o�R�i��n2")+"::OnStart";
	stopnpctimer;
	end;
}

1@ma_b.gat,36,111,4	script	#�o�R�i��n2	844,{
	end;
OnStart:
	set '@label$, getmdnpcname("#�o�R�i��n2")+"::OnKilled";
	set '@map$, getmdmapname("1@ma_b.gat");
	monster '@map$,95,98,"��",2328,1,'@label$;
	monster '@map$,60,98,"��",2328,1,'@label$;
	monster '@map$,97,104,"����",2328,1,'@label$;
	monster '@map$,58,104,"����",2328,1,'@label$;
	donpcevent getmdnpcname("#�o�R�i��n2-1")+"::OnStart";
	initnpctimer;
	end;
OnKilled:
	set '@count,getmapmobs(getmdmapname("1@ma_b.gat"),getmdnpcname("#�o�R�i��n2")+"::OnKilled");
	if('@count < 1) {
		donpcevent getmdnpcname("#�o�R�i��n2-1")+"::OnEnd";
		stopnpctimer;
	}
	else
		announce "�^�z�[�F���������ł��A����"+'@count+"�ł��I",0x9,0x00ffff,0x190,15;
	end;
OnTimer1000:	callsub OnAnnounce,"5��",15,1;
OnTimer60000:	callsub OnAnnounce,"4��",15,1;
OnTimer120000:	callsub OnAnnounce,"3��",15,1;
OnTimer180000:	callsub OnAnnounce,"2��",15,1;
OnTimer240000:	callsub OnAnnounce,"1��",15,1;
OnTimer270000:	callsub OnAnnounce,"30�b",15,0;
OnTimer280000:	callsub OnAnnounce,"20�b",15,0;
OnTimer290000:	callsub OnAnnounce,"10�b",15,0;
OnTimer295000:	callsub OnAnnounce,"5�b",16,0;
OnTimer296000:	callsub OnAnnounce,"4�b",17,0;
OnTimer297000:	callsub OnAnnounce,"3�b",18,0;
OnTimer298000:	callsub OnAnnounce,"2�b",19,0;
OnTimer299000:	callsub OnAnnounce,"1�b",20,0;
OnTimer300000:
	announce "�o�R�i�����΂̒��ɓ����čs���܂����B",0x9,0xffff00,0x190,20;
	donpcevent getmdnpcname("�^�z�[#���s")+"::OnStart";
	killmonster getmdmapname("1@ma_b.gat"),getmdnpcname("#�o�R�i��n2")+"::OnKilled";
	stopnpctimer;
	end;
OnAnnounce:
	announce "��������"+getarg(0),0x9,0xff4400,0x190,getarg(1);
	if(getarg(2)) donpcevent getmdnpcname("#�艺����n1")+"::OnStart";
	end;
}

1@ma_b.gat,36,111,4	script	#�o�R�i��n2-1	844,{
	end;
OnInstanceInit:
	disablenpc getmdnpcname("#�o�R�i��n2-1");
	end;
OnStart:
	enablenpc getmdnpcname("#�o�R�i��n2-1");
	monster getmdmapname("1@ma_b.gat"),78,93,"�o�R�i��",2321,1,getmdnpcname("#�o�R�i��n2-1")+"::OnKilled";
	end;
OnEnd:
	enablenpc getmdnpcname("#�o�R�i��n2-1");
	killmonster getmdmapname("1@ma_b.gat"),getmdnpcname("#�o�R�i��n2-1")+"::OnKilled";
	initnpctimer;
	end;
OnTimer1000:
	announce "�^�z�[�F�o�R�i�������̒��ɉB��Ă��܂��܂����ˁB����ŏI������̂��ȁH",0x9,0x00ffff,0x190,15;
	end;
OnTimer5000:
	announce "�^�z�[�F�����A���̒��ŉ����������Ă��܂��I�@������܂��I",0x9,0x00ffff,0x190,15;
	end;
OnTimer10000:
	announce "�ڕW�]����ɋ��\�ɂȂ����o�R�i����|�����Ɓ]",0x9,0xff3300,0x190,15;
	donpcevent getmdnpcname("#�o�R�i��n3")+"::OnStart";
	stopnpctimer;
	end;
OnKilled:
	end;
}

1@ma_b.gat,36,111,4	script	#�o�R�i��n3	844,{
	end;
OnStart:
	initnpctimer;
	monster getmdmapname("1@ma_b.gat"),78,83,"Enraged Bakonawa",2322,1,getmdnpcname("#�o�R�i��n3")+"::OnKilled";
	donpcevent getmdnpcname("#�o�R�i��n3-1")+"::OnStart";
	end;
OnKilled:
	announce "�^�z�[�F�o�R�i����ގ����܂����I�@����ł��΂炭�̊Ԃ͕����ɕ�炷���Ƃ��ł��܂��B",0x9,0x00ffff,0x190,15;
	donpcevent getmdnpcname("�^�z�[#����")+"::OnStart";
	donpcevent getmdnpcname("#�o�R�i��n3-1")+"::OnEnd";
	stopnpctimer;
	end;
OnTimer1000:
	callsub OnAnnounce,"10��",15,1;
	end;
OnTimer60000:
OnTimer120000:
OnTimer180000:
OnTimer240000:
	donpcevent getmdnpcname("#�艺����n1")+"::OnStart";
	end;
OnTimer300000:	callsub OnAnnounce,"5��",15,1;
OnTimer360000:	callsub OnAnnounce,"4��",15,1;
OnTimer420000:	callsub OnAnnounce,"3��",15,1;
OnTimer480000:	callsub OnAnnounce,"2��",15,1;
OnTimer540000:	callsub OnAnnounce,"1��",15,1;
OnTimer570000:	callsub OnAnnounce,"30�b",15,0;
OnTimer580000:	callsub OnAnnounce,"20�b",15,0;
OnTimer590000:	callsub OnAnnounce,"10�b",15,0;
OnTimer595000:	callsub OnAnnounce,"5�b",16,0;
OnTimer596000:	callsub OnAnnounce,"4�b",17,0;
OnTimer597000:	callsub OnAnnounce,"3�b",18,0;
OnTimer598000:	callsub OnAnnounce,"2�b",19,0;
OnTimer599000:	callsub OnAnnounce,"1�b",20,0;
OnTimer600000:
	announce "�o�R�i�����΂̒��ɓ����čs���܂����B",0x9,0xffff00,0x190,20;
	donpcevent getmdnpcname("�^�z�[#���s")+"::OnStart";
	killmonster getmdmapname("1@ma_b.gat"),getmdnpcname("#�o�R�i��n3")+"::OnKilled";
	stopnpctimer;
	end;
OnAnnounce:
	announce "��������"+getarg(0),0x9,0xff4400,0x190,getarg(1);
	if(getarg(2)) donpcevent getmdnpcname("#�艺����n1")+"::OnStart";
	end;
}

1@ma_b.gat,36,111,4	script	#�o�R�i��n3-1	844,{
	end;
OnStart:
	initnpctimer;
	end;
OnEnd:
	killmonster getmdmapname("1@ma_b.gat"),getmdnpcname("#�o�R�i��n3-1")+"::OnKilled";
	stopnpctimer;
	end;
OnTimer120000:	callsub OnMobSpawn,10;
OnTimer180000:	callsub OnMobSpawn,15;
OnTimer240000:	callsub OnMobSpawn,20;
//OnTimer300000:	callsub OnMobSpawn,25;
OnTimer300000:	callsub OnMobSpawn,30;
OnTimer360000:	callsub OnMobSpawn,35;
OnTimer420000:	callsub OnMobSpawn,40;
OnTimer480000:	callsub OnMobSpawn,45;
OnTimer540000:	callsub OnMobSpawn,50;
OnTimer600000:
	killmonster getmdmapname("1@ma_b.gat"),getmdnpcname("#�o�R�i��n3-1")+"::OnKilled";
	stopnpctimer;
	end;
OnMobSpawn:
	set '@label$, getmdnpcname("#�o�R�i��n3-1")+"::OnKilled";
	set '@map$, getmdmapname("1@ma_b.gat");
	killmonster '@map$,'@label$;
	areamonster '@map$,74,74,82,74,"�o�R�i���̎艺",2334,getarg(0),'@label$;
	end;
OnKilled:
	end;
}

1@ma_b.gat,66,64,0	script	�^�z�[#����	541,{
	mes "[�^�z�[]";
	mes "�������킢�ł����ˁI";
	mes "�艺���c���Ă�����댯�Ȃ̂ŁA";
	mes "��ɖ߂�܂��傤�B";
	next;
	mes "[�^�z�[]";
	mes "���I";
	mes "�O�Ŏ��ɘb�������Ă��ꂽ��A";
	mes "����������グ�܂��ˁI";
	close2;
	warp "ma_scene01.gat",175,176;
	end;
OnStart:
	hideoffnpc getmdnpcname("�^�z�[#����");
	initnpctimer;
	end;
OnTimer1000:
	monster getmdmapname("1@ma_b.gat"),78,74,"�o�R�i���̕�",2335,1;
	end;
OnTimer10000:
	announce "�^�z�[�F�󔠂��J������A���̏��ɗ��Ă��������B�n��������������܂��B",0x9,0x00ffff,0x190,15;
	stopnpctimer;
	end;
}

1@ma_b.gat,61,52,4	script	�^�z�[#���s	541,{
	mes "[�^�z�[]";
	mes "��蓦�����Ă��܂��܂����ˁB";
	mes "�΂̒�ɐ����Ă��܂��ƁA";
	mes "�����炩���o���ł��܂���B";
	next;
	if(getpartyleader(getcharid(1)) == strcharinfo(0)) {
		mes "[�^�z�[]";
		mes "������x���킵�܂����H";
		next;
		switch(select("������Ƒ҂���","������x�Ăяo���Ă���","������߂ċA��")) {
		case 1:
			mes "[�^�z�[]";
			mes "���c�c��Ȃ�����Ȃ��ł����I";
			mes "���₤���Ăяo�����ł�����B";
			close;
		case 2:
			mes "[�^�z�[]";
			mes "�ł́A���炽�߂�";
			mes "����Ȗ���΂ɗ��Ƃ��Ă݂܂��B";
			next;
			mes "[�^�z�[]";
			mes "���͉B��܂��B";
			mes "�܂���10���ȓ���";
			mes "�o�R�i����|���Ă��������I";
			donpcevent getmdnpcname("#�o�R�i��n1")+"::OnStart";
			hideonnpc getmdnpcname("�^�z�[#���s");
			close;
		case 3:
			mes "[�^�z�[]";
			mes "���O�ł��B";
			mes "�r�𖁂��āA����͑ގ����܂��傤�B";
			next;
			mes "[�^�z�[]";
			mes "����ł́A�߂�܂��傤�B";
			close2;
			warp "ma_scene01.gat",175,176;
			end;
		}
	}
	else {
		mes "[�^�z�[]";
		mes "������x���킷�邩";
		mes "���[�_�[�Ƒ��k���Ă��������B";
		next;
		if(select("������x�Ăяo���Ă���","������߂ċA��") == 1) {
			mes "[�^�z�[]";
			mes "����ł́A���[�_�[����";
			mes "�b��������悤�`���Ă��������B";
			close;
		}
		mes "[�^�z�[]";
		mes "���O�ł��B";
		mes "�r�𖁂��āA����͑ގ����܂��傤�B";
		next;
		mes "[�^�z�[]";
		mes "����ł́A�߂�܂��傤�B";
		close2;
		warp "ma_scene01.gat",175,176;
		end;
	}
OnStart:
	hideoffnpc getmdnpcname("�^�z�[#���s");
	end;
}

//1@ma_b.gat,62,51,0	warp	#�o�R�i�������e���|	2,2,ma_scene01.gat,175,176