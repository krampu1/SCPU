
//  название, номер, есть ли аргументы, что делать

#define PUSH(arg) stack_push(&(cpu->stack), arg)

#define POP       pop(cpu)

#define PUSH_CALL(arg) stack_push(&(cpu->calls), arg)

#define POP_CALL       stack_pop(&(cpu->calls))


DEF_CMD(push, 1, 1, {   int *arg = get_ptr_arg(cpu);

                        PUSH(*arg);
                    })

DEF_CMD(add, 2, 0, {    PUSH(POP + POP);
                    })

DEF_CMD(sup, 3, 0, {    int b = POP;
                        int a = POP;

                        PUSH(a - b);
                    })

DEF_CMD(mul, 4, 0, {    PUSH(POP * POP);
                    })

DEF_CMD(div, 5, 0, {    int b = POP;
                        int a = POP;

                        if (b == 0) {
                            PUSH(std::numeric_limits<int>::max());
                        }
                        else {
                            PUSH(a / b);
                        }
                    })

DEF_CMD(out, 6, 0, {    printf("%d\n", POP);
                    })

DEF_CMD(hell, 7, 0, {   end_flug = true;
                        break;
                    })

DEF_CMD(jmp, 8, 1, {    int *arg = get_ptr_arg(cpu);

                        cpu->ip = *arg;
                    })

DEF_CMD(pop, 9, 1, {    int *arg = get_ptr_arg(cpu);

                        *arg = POP;
                    })

DEF_CMD(call, 10, 1, {  int *arg = get_ptr_arg(cpu);

                        PUSH_CALL((int)cpu->ip);
                        
                        cpu->ip = *arg;
                    })

DEF_CMD(ret, 11, 0, {   if (cpu->calls.size == 0) {
                            fprintf(stderr, "error ret ip:%Iu\n", cpu->ip);
                            end_flug = true;
                            break;
                        }
                        cpu->ip = POP_CALL;
                    })

DEF_CMD(jb, 12, 1, {    int a = 0;
                        int b = 0;
                        int arg = 0;

                        get_jmp_param(&a, &b, &arg, cpu);
    
                        if (a < b) {
                            cpu->ip = arg;
                        }
                    })
#ifdef GRAPHICS
DEF_CMD(drom, 13, 0, {  draw_window(&window, cpu, &pointmap);
                     })
#endif

DEF_CMD(sqrt, 14, 0, {  PUSH((int)sqrt(POP));
                     })

DEF_CMD(je, 15, 1, {    int a = 0;
                        int b = 0;
                        int arg = 0;

                        get_jmp_param(&a, &b, &arg, cpu);
    
                        if (a == b) {
                            cpu->ip = arg;
                        }
                    })

DEF_CMD(nroot, 16, 0, { printf("No root\n");
                      })

DEF_CMD(infroot, 17, 0, {   printf("Inf root\n");
                        })