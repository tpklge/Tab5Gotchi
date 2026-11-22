# Continuidade — revisão 0.2.1

A revisão nova é instalada pelo launcher no slot 2. Slot 0 (Meshtastic) e slot 1 anterior são preservados. Saves passaram a v2; não abra progresso novo com o aplicativo antigo do slot 1. Os backups antes das etapas estão em private/hardware/stage-* e os binários substituídos têm backup do próprio launcher no SD. Não instale bootloader/tabela gerados pelo projeto.

As pendências de software auditadas foram tratadas nesta revisão; veja status.md, architecture.md, requirements.md e os relatórios de testes. Pacotes são regenerados por tools/package.py, incluindo checksums e sem progresso pessoal.

Pendências de validação física:

1. Medir autonomia e comparar tensão/porcentagem aproximada com instrumento e diferentes estados de carga. Não há previsão de autonomia no jogo.
2. Verificar RTC após retirada total de alimentação.
3. Ensaiar cortes controlados em diferentes fases de gravação com perfil de teste e backup. O usuário já confirmou alguns reinícios aleatórios pelo botão, teclado e touch.
4. Validar visitas/presentes da variante extras entre dois Tab5. O protocolo tem teste entre dois processos host; não há integração física entre dois aparelhos comprovada.
5. Avaliar audição/qualidade acústica. Contadores do codec passaram; isso não comprova a percepção sonora humana.

Para novo conteúdo, use tools/generate_assets.py (inclui assets_src/enhance.py) e tools/validate_assets.py. Preserve IDs e diretórios saves/album. Para mudança de payload persistente, acrescente migração explícita e testes antes da próxima instalação.
