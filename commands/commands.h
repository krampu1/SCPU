
//  название, номер, есть ли аргументы, что делать

DEF_CMD(push, 1, 1, {   int *arg = get_ptr_arg(cpu);

                        stack_push(&(cpu->stack), *arg);

                        break;
                    })

DEF_CMD(add, 2, 0, {    if (cpu->stack.size < 2) {
                            printf("ERROR in add ip:%d, stack size < 2\n", (cpu->ip)-2);
                            end_flug = true;
                            break;
                        }

                        stack_push(&(cpu->stack), stack_pop(&(cpu->stack)) + stack_pop(&(cpu->stack)));

                        break;
                    })

DEF_CMD(sup, 3, 0, {    if (cpu->stack.size < 2) {
                            printf("ERROR in sup ip:%d, stack size < 2\n", (cpu->ip)-2);
                            end_flug = true;
                            break;
                        }

                        int b = stack_pop(&(cpu->stack));
                        int a = stack_pop(&(cpu->stack));

                        stack_push(&(cpu->stack), a - b);

                        break;
                    })

DEF_CMD(mul, 4, 0, {    if (cpu->stack.size < 2) {
                            printf("ERROR in mul ip:%d, stack size < 2\n", (cpu->ip)-2);
                            end_flug = true;
                            break;
                        }

                        stack_push(&(cpu->stack), stack_pop(&(cpu->stack)) * stack_pop(&(cpu->stack)));

                        break;
                    })

DEF_CMD(div, 5, 0, {    if (cpu->stack.size < 2) {
                            printf("ERROR in div ip:%d, stack size < 2\n", (cpu->ip)-2);
                            end_flug = true;
                            break;
                        }

                        int b = stack_pop(&(cpu->stack));
                        int a = stack_pop(&(cpu->stack));

                        if (b == 0) {
                            stack_push(&(cpu->stack), std::numeric_limits<int>::max());
                        }
                        else {
                            stack_push(&(cpu->stack), a / b);
                        }

                        break;
                    })

DEF_CMD(out, 6, 0, {    if (cpu->stack.size < 1) {
                            printf("ERROR in out ip:%d, stack is empty\n", (cpu->ip)-2);
                            end_flug = true;
                            break;
                        }

                        printf("%d\n", stack_pop(&(cpu->stack)));

                        break;
                    })

DEF_CMD(hell, 7, 0, {   end_flug = true;
                        break;
                    })

DEF_CMD(jmp, 8, 1, {    int *arg = get_ptr_arg(cpu);

                        cpu->ip = *arg;

                        break;
                    })

DEF_CMD(pop, 9, 1, {    int *arg = get_ptr_arg(cpu);

                        *arg = stack_pop(&(cpu->stack));

                        break;
                    })

DEF_CMD(call, 10, 1, {  char cmd_param = cpu->program[(cpu->ip)++];
                        int inte = 0;

                        if (cmd_param & REG_MASK) {
                            inte  += (unsigned char)cpu->REG[cpu->program[(cpu->ip)++]];
                        }

                        if (cmd_param & INT_MASK) {
                            int inte_read = 0;
                            for (size_t i = 0; i < sizeof(int); i++) {
                                ((char *)(&inte_read))[i] = cpu->program[(cpu->ip)++];
                            }

                            inte += inte_read;
                        }

                        stack_push(&(cpu->calls), cpu->ip);
                        
                        cpu->ip = inte;
                        break;
                    })

DEF_CMD(ret, 11, 0, {   if (cpu->calls.size == 0) {
                            printf("error ret ip:%d\n", cpu->ip);
                            end_flug = true;
                            break;
                        }
                        cpu->ip = stack_pop(&(cpu->calls));
                        break;
                    })

DEF_CMD(jb, 12, 1, {    int a = 0;
                        int b = 0;
                        int arg = 0;

                        get_jmp_param(&a, &b, &arg, cpu);
    
                        if (a < b) {
                            cpu->ip = arg;
                        }

                        break;
                    })

DEF_CMD(drom, 13, 0, {  /*for (int i = 0; i < 1000000; i++) {
                            if (i%1000==0) {
                                printf("\n");
                            }
                            if (RAM[i]) {
                                printf("*");
                            }
                            else{
                                printf(".");
                            }
                            
                        }

                        printf("\n");*/
                        if (window.isOpen()) {
                            for (register int a = 0; a < 320*4 * 200*4 ; a++) {
                                pointmap[a].position = sf::Vector2f((a % (320 * 4)), ((a/(320 * 4))%(200 * 4)));
                                pointmap[a].color = (sf::Color)(cpu->RAM[a % (320 * 4) / 4 + ((a/(320 * 4))%(200 * 4)) / 4 * 320]);
                            }
                            window.clear();
                            window.draw(pointmap);

                            window.display();
                        }

                        break;
                     })

DEF_CMD(sqrt, 14, 0, {  if (cpu->stack.size < 1) {
                            printf("stack empty, sqrt, ip:%d\n", cpu->ip);
                            end_flug = true;
                            break;
                        }

                        stack_push(&(cpu->stack), sqrt(stack_pop(&(cpu->stack))));

                        break;
                     })

DEF_CMD(je, 15, 1, {    int a = 0;
                        int b = 0;
                        int arg = 0;

                        get_jmp_param(&a, &b, &arg, cpu);
    
                        if (a == b) {
                            cpu->ip = arg;
                        }

                        break;
                    })