# sd-project3
Pequeno memory leak.
Não há outras limitações aparentes no projeto
Membros do grupo:
João Santos, 56380
Marcos Gomes, 56326
Pedro Trindade, 56342


escrita na cabeca
leituras na cauda


mandar mail prof a ver se
	podemos mudar assinatura do tree_skel_init?
	ou se temos de ter variaveis globais (usar extern?)

	posso dar import ao tree_server no tree_skel?

	como obter o ip externo do servidor?


# ZooKeeper
Lançar ZooKeeper ✅

# Servidor
Servidor ligar-se ao ZooKeeper ✅
Servidor verificar (e criar) root node/ZNode: "/chain" ✅
Servidor registar-se no ZooKeeper sob "/chain" ✅
Guardar ID que o ZooKeeper lhe atribuiu ✅
Fazer watch a "/chain" (vamos ser notificados se algum filho de /chain mudar) ✅
Ver qual o ID do próximo servidor ao qual nos temos que ligar ✅
Pedir ao ZooKeeper para nos dar o IP:PORT desse servidor ✅
Guardar e ligar a esse servidor como next_server ✅

make ssserver_run2 args="1337"
make ssserver_run2 args="1338"
make ssserver_run2 args="1339"
make ssserver_run2 args="1340"

# Cliente
⚠️

