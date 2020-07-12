# Sistemas-Operacionais-
Projetos produzidos durante o curso de Sistemas Operacionais.

Experimento 1->

Introdução:

O primeiro experimento permite o contato com dois assuntos importantes em Sistemas
Operacionais: a criação de processos e o conceito de tempo. Este experimento engloba
vários fatores importantes que precisam ser percebidos sobre a duração da execução de
um programa em um ambiente multitarefa.


Experimento 2->

Introdução:

O experimento #1 deve ter permitido o aprendizado sobre criação de processos
concorrentes no Sistema Operacional (SO) Unix. No entanto, existem aplicações que
requerem algum método que permita o compartilhamento de dados (informação) entre
processos.
IPC (Inter-Process Communication), ou comunicação entre processos, consiste em um
conjunto de métodos que permitem que processos em um SO Unix possam se comunicar.
Como existem muitas versões de Unix, existem métodos diferentes para realização de
IPC. A variante Berkeley do Unix usa sockets. A variante da AT&T, o System V, usa filas de
mensagens e compartilhamento de memória. Além disso, no POSIX.1b, filas de
mensagens e memória compartilhada são definidas de modo diferente que no System V.
Pipes constituem outro tipo de mecanismo para IPC. Até mesmo um arquivo pode ser
usado para comunicação entre processos. Apesar da variedade de métodos diferentes
para IPC, é importante entender e perceber como processos podem se comunicar.

Experimento 3->

Introdução:

No experimento anterior foi utilizado um mecanismo de IPC (Inter-Process
Communication) baseado em fila de mensagens. Usando este mecanismo foi possível
compartilhar dados entre diferentes processos. Outro mecanismo de IPC é a memória
compartilhada. Quando uma região de memória é compartilhada entre dois processos que
podem ter acesso “simultaneamente” para modificação por, pelo menos, um deles, esta
região de memória deve ser protegida.
Um mecanismo que pode ser usado para solucionar o problema de acesso simultâneo a
uma mesma área de memória, se chama semáforo. Semáforos são mecanismos
compartilháveis entre processos que garantem sincronismo, se utilizados de maneira
adequada, por exemplo, para que um processo tenha acesso exclusivo a um recurso
crítico em uma localização de memória.
Neste experimento, deseja-se explorar o conceito de exclusão mútua e perceber o
funcionamento de uma implementação de semáforos.

Experimento 4->

Introdução:

Nos experimentos prévios foram utilizados mecanismos de IPC (Inter-Process
Communication) baseados em fila de mensagens, memória compartilhada e semáforos.
Usando estes mecanismos, foi possível compartilhar dados entre diferentes processos.
Processos, naturalmente, são independentes entre si em relação aos seus espaços de
endereçamento de dados e aos seus contextos, enquanto compartilham uma ou mais
CPUs do sistema, de maneira concorrente. Para compartilhar dados, processos
necessitam de mecanismos de IPC para seu acesso e também para que esse acesso
ocorra sob exclusão mútua, se houver condição de corrida.
Uma outra maneira de haver concorrência é através do uso de threads. Uma thread é
definida como uma linha (sequência) de execução que pode ser concorrente a outras
linhas, dentro de um mesmo processo, ao mesmo tempo em que é capaz de acessar
recursos comuns a todas as linhas desse processo.
Todo processo (programa em execução) é por definição uma thread, que correspondente
à rotina main(). Uma vez em execução, esta primeira thread pode criar outras threads que
concorrerão com ela pela(s) CPU(s). Uma thread deverá corresponder à execução de
uma rotina declarada no programa e disparada através de chamada ao sistema
apropriada.
Uma thread, também denominada de processo-leve, passa a ser a unidade de
escalonamento dentro do SO, com uma possível e condicional vantagem sobre o
escalonamento baseado em processos: a de uma troca de contexto mais simples e,
consequentemente, mais rápida. Além disso, o fato das threads de um mesmo processo
poderem acessar os recursos comuns não requer, necessariamente, o uso de
mecanismos de IPC para uma comunicação entre as mesmas.

Experimento 5->

Introdução:

O IPC (Inter-Process Communication), ou comunicação entre processos, consiste de um
conjunto de métodos que permitem que processos no Sistema Operacional Unix possam
se comunicar. Em um dos experimentos anteriores foram exploradas as chamadas para
alguns mecanismos de IPC existentes no System V, mais especificamente as filas de
mensagens.
Neste experimento, deseja-se que filas de mensagens sejam usadas para propiciar
sincronismo no problema do Barbeiro Dorminhoco. Deseja-se também que a solução
desse problema seja implementada com threads.
