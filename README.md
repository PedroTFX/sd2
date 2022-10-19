# sd-project2

For a better understaing of the problem:

client-server   ->    client_stub    ->    network_client
                                       |server_communication|
                     tree-server     ->   network_server  <-    tree_skel   <- projeto1
 

Algumas dicas Uteis na realização de codigo:
     
  - Recomenda-se  a  criação  de  funções  read_all  e  write_all  que  vão  receber  e  enviar 
strings inteiras pela rede (lembrar que as funções read/write em sockets nem sempre 
leem/escrevem tudo o que pedimos). Um bom sítio para concretizar essas funções é 
num modulo separado a ser incluído pelo cliente e servidor ou no message-private.h 
(concretizando-as no message.c).  

  - Usar a função signal() para ignorar sinais do tipo SIGPIPE, lançados quando uma das 
pontas comunicantes fecha o socket de maneira inesperada. Isto deve ser feito tanto no 
cliente quanto no servidor, evitando que um programa termine abruptamente (crash) 
quando a outra parte é desligada. 
    
  - Usar  a  função  setsockopt(...,  SO_REUSEADDR,  ...)  para  fazer  com  que  o  servidor 
consiga fazer bind a um porto usado anteriormente e registado pelo kernel como ainda 
ocupado. Isto permite que o servidor seja reinicializado rapidamente, sem ter de esperar 
o tempo de limpeza da árvore de portos usados, mantida pelo kernel. 
    
  - Caso algum dos pedidos não possa ser atendido devido a um erro, o servidor vai retornar 
{OP_ERROR, CT_RESULT, errcode} ou {OP_ERROR, CT_NONE} (dependendo do 
pedido), onde errcode é o código do erro retornado ao executar a operação na árvore 
do servidor (em geral, -1).  
Note que o caso em que uma chave não é encontrada no get não deve ser considerado 
como  erro.  Neste  caso  o  servidor  deve  responder  com  uma  resposta  normal 
(OP_GET+1) mas definindo um data_t com size=0 e data=NULL.

Melhor metodo de teste:

 • correr (sim / não)
 • tem leaks? (sim / não)
 • tem erros do valgrind? (sim / não)
