cmd_/home/dennis/Sistemaoperativos/e-SO1_2S2024_201700747/proyecto1/modulos/Module.symvers := sed 's/\.ko$$/\.o/' /home/dennis/Sistemaoperativos/e-SO1_2S2024_201700747/proyecto1/modulos/modules.order | scripts/mod/modpost -m -a  -o /home/dennis/Sistemaoperativos/e-SO1_2S2024_201700747/proyecto1/modulos/Module.symvers -e -i Module.symvers   -T -