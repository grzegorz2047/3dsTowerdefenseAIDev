from pathlib import Path


def replace(path, old, new):
    p = Path(path)
    text = p.read_text()
    if text.count(old) != 1:
        raise RuntimeError(f'{path}: expected one match, got {text.count(old)}: {old[:60]!r}')
    p.write_text(text.replace(old, new, 1))


# Preserve explicit physical-button hints while every result button remains touchable.
p = Path('source/UiRenderer.cpp')
text = p.read_text().replace('"KAMPANIA", true);', '"X KAMPANIA", true);')
text = text.replace('"POWTORZ", false);', '"Y POWTORZ", false);')
p.write_text(text)

replace('include/Renderer.hpp', '#include <cstddef>\n#include <cstdint>', '#include <array>\n#include <cstddef>\n#include <cstdint>')
replace('include/Renderer.hpp', '''    std::size_t towerVertexOffset_ = 0;
    std::size_t towerVertexCount_ = 0;
    std::size_t projectileVertexOffset_ = 0;
    std::size_t projectileVertexCount_ = 0;''', '''    std::array<std::size_t, 4U> towerVertexOffsets_{};
    std::array<std::size_t, 4U> towerVertexCounts_{};
    std::size_t projectileVertexOffset_ = 0U;
    std::size_t projectileVertexCount_ = 0U;
    std::size_t rocketVertexOffset_ = 0U;
    std::size_t rocketVertexCount_ = 0U;''')

replace('source/Renderer.cpp', '''void appendTower(std::vector<Vertex>& vertices) {
    appendBox(vertices, 0.0F, 0.0F, 0.42F, 0.42F, 0.06F, 0.22F, 0.27F, 0.32F, 0.39F);
    appendBox(vertices, 0.0F, 0.0F, 0.31F, 0.31F, 0.20F, 0.88F, 0.43F, 0.49F, 0.57F);
    appendBox(vertices, 0.0F, 0.0F, 0.39F, 0.39F, 0.86F, 1.00F, 0.27F, 0.32F, 0.39F);
    appendBox(vertices, 0.0F, -0.33F, 0.08F, 0.30F, 0.92F, 1.04F, 0.42F, 0.24F, 0.10F);
}

void appendProjectile(std::vector<Vertex>& vertices) {
    appendBox(vertices, 0.0F, 0.0F, 0.06F, 0.06F, -0.06F, 0.06F, 0.94F, 0.76F, 0.24F);
}''', '''void appendBallistaTower(std::vector<Vertex>& v) {
    appendBox(v,0,0,.42F,.42F,.05F,.22F,.25F,.31F,.38F);
    appendBox(v,0,0,.12F,.12F,.20F,.78F,.47F,.31F,.14F);
    appendBox(v,0,-.18F,.48F,.07F,.70F,.82F,.67F,.45F,.20F);
    appendBox(v,0,-.52F,.04F,.42F,.73F,.79F,.20F,.16F,.12F);
}
void appendMortarTower(std::vector<Vertex>& v) {
    appendBox(v,0,0,.45F,.45F,.05F,.25F,.29F,.33F,.38F);
    appendBox(v,0,0,.32F,.32F,.24F,.52F,.38F,.40F,.42F);
    appendRotatedBox(v,0,-.18F,.17F,.40F,.48F,.76F,-.18F,.18F,.20F,.22F);
    appendBox(v,0,-.57F,.13F,.18F,.58F,.82F,.10F,.11F,.12F);
}
void appendFrostTower(std::vector<Vertex>& v) {
    appendBox(v,0,0,.43F,.43F,.04F,.20F,.18F,.34F,.51F);
    appendBox(v,0,0,.25F,.25F,.18F,.85F,.23F,.67F,.83F);
    appendRotatedBox(v,0,0,.08F,.50F,.50F,.94F,.78F,.65F,.91F,1.0F);
    appendRotatedBox(v,0,0,.50F,.08F,.50F,.94F,.78F,.65F,.91F,1.0F);
}
void appendRocketTower(std::vector<Vertex>& v) {
    appendBox(v,0,0,.46F,.46F,.04F,.22F,.29F,.31F,.35F);
    appendBox(v,0,0,.29F,.29F,.20F,.66F,.43F,.18F,.13F);
    appendBox(v,-.17F,-.18F,.11F,.36F,.58F,.82F,.18F,.20F,.22F);
    appendBox(v,.17F,-.18F,.11F,.36F,.58F,.82F,.18F,.20F,.22F);
    appendBox(v,-.17F,-.58F,.08F,.12F,.61F,.79F,.80F,.24F,.12F);
    appendBox(v,.17F,-.58F,.08F,.12F,.61F,.79F,.80F,.24F,.12F);
}
void appendProjectile(std::vector<Vertex>& v) {
    appendBox(v,0,0,.06F,.06F,-.06F,.06F,.94F,.76F,.24F);
}
void appendRocketProjectile(std::vector<Vertex>& v) {
    appendBox(v,0,0,.07F,.25F,-.07F,.07F,.72F,.72F,.75F);
    appendBox(v,0,-.29F,.065F,.08F,-.065F,.065F,.91F,.22F,.13F);
    appendBox(v,0,.31F,.05F,.12F,-.05F,.05F,1.0F,.62F,.10F);
    appendBox(v,0,.52F,.035F,.18F,-.035F,.035F,1.0F,.26F,.05F);
}
void appendBenchmarkDecoration(std::vector<Vertex>& v, std::size_t i, float x, float z) {
    switch (i % 6U) {
        case 0: appendBox(v,x,z,.07F,.07F,0,.48F,.30F,.18F,.08F); appendBox(v,x,z,.30F,.30F,.42F,.86F,.16F,.38F,.17F); break;
        case 1: appendRotatedBox(v,x,z,.34F,.25F,0,.33F,.35F,.36F,.35F,.34F); break;
        case 2: appendBox(v,x,z,.38F,.10F,0,.70F,.43F,.41F,.36F); appendBox(v,x+.25F,z,.10F,.28F,0,.38F,.34F,.33F,.30F); break;
        case 3: appendBox(v,x,z,.035F,.035F,0,.92F,.28F,.18F,.09F); appendBox(v,x+.15F,z,.16F,.025F,.56F,.82F,.70F,.12F,.15F); break;
        case 4: appendRotatedBox(v,x,z,.45F,.07F,.10F,.27F,.55F,.42F,.25F,.10F); break;
        default: appendBox(v,x,z,.22F,.22F,0,.72F,.40F,.38F,.33F); appendBox(v,x,z,.35F,.35F,.70F,.84F,.24F,.28F,.31F); break;
    }
}''')

replace('source/Renderer.cpp', '''    if (artResult.success && artResult.art.levelId == level.id) {
        for (std::size_t index = 0U; index < artResult.art.propCount; ++index) {
            appendProp(vertices, artResult.art.props[index], offsetX, offsetZ);
        }
    }
    if (vertices.size() > PerformanceBudget::kMaximumLevelVertices) return false;''', '''    if (artResult.success && artResult.art.levelId == level.id) {
        for (std::size_t index = 0U; index < artResult.art.propCount; ++index) appendProp(vertices, artResult.art.props[index], offsetX, offsetZ);
    }
    if (level.id == "performance_stress") {
        for (std::size_t i=0; i<benchmarkDecorations_; ++i) {
            const std::size_t column=i % static_cast<std::size_t>(level.width);
            const std::size_t band=i / static_cast<std::size_t>(level.width);
            const float x=offsetX+static_cast<float>(column);
            const float z=band%2U==0U ? offsetZ+.45F : offsetZ+static_cast<float>(level.height-1U)-.45F;
            appendBenchmarkDecoration(vertices,i,x,z);
        }
    }
    if (vertices.size() > PerformanceBudget::kMaximumLevelVertices) return false;''')
replace('source/Renderer.cpp', '''    levelVertexCount_ = vertices.size();
    enemyVertexOffset_ = vertices.size(); appendEnemy(vertices); enemyVertexCount_ = vertices.size() - enemyVertexOffset_;
    towerVertexOffset_ = vertices.size(); appendTower(vertices); towerVertexCount_ = vertices.size() - towerVertexOffset_;
    projectileVertexOffset_ = vertices.size(); appendProjectile(vertices); projectileVertexCount_ = vertices.size() - projectileVertexOffset_;''', '''    levelVertexCount_ = vertices.size();
    enemyVertexOffset_=vertices.size(); appendEnemy(vertices); enemyVertexCount_=vertices.size()-enemyVertexOffset_;
    towerVertexOffsets_[0]=vertices.size(); appendBallistaTower(vertices); towerVertexCounts_[0]=vertices.size()-towerVertexOffsets_[0];
    towerVertexOffsets_[1]=vertices.size(); appendMortarTower(vertices); towerVertexCounts_[1]=vertices.size()-towerVertexOffsets_[1];
    towerVertexOffsets_[2]=vertices.size(); appendFrostTower(vertices); towerVertexCounts_[2]=vertices.size()-towerVertexOffsets_[2];
    towerVertexOffsets_[3]=vertices.size(); appendRocketTower(vertices); towerVertexCounts_[3]=vertices.size()-towerVertexOffsets_[3];
    projectileVertexOffset_=vertices.size(); appendProjectile(vertices); projectileVertexCount_=vertices.size()-projectileVertexOffset_;
    rocketVertexOffset_=vertices.size(); appendRocketProjectile(vertices); rocketVertexCount_=vertices.size()-rocketVertexOffset_;''')
replace('source/Renderer.cpp', '''        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, modelViewUniform_, &model);
        C3D_DrawArrays(GPU_TRIANGLES, static_cast<int>(towerVertexOffset_), static_cast<int>(towerVertexCount_));
    }
    if (level_ != nullptr) {''', '''        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, modelViewUniform_, &model);
        const std::size_t type=static_cast<std::size_t>(tower.type());
        C3D_DrawArrays(GPU_TRIANGLES,static_cast<int>(towerVertexOffsets_[type]),static_cast<int>(towerVertexCounts_[type]));
    }
    if (level_ != nullptr) {''')
replace('source/Renderer.cpp', '''        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, modelViewUniform_, &cursor);
        C3D_DrawArrays(GPU_TRIANGLES, static_cast<int>(towerVertexOffset_), static_cast<int>(towerVertexCount_));
    }''', '''        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, modelViewUniform_, &cursor);
        const std::size_t selected=static_cast<std::size_t>(buildSystem.selectedTowerType());
        C3D_DrawArrays(GPU_TRIANGLES,static_cast<int>(towerVertexOffsets_[selected]),static_cast<int>(towerVertexCounts_[selected]));
    }''')
replace('source/Renderer.cpp', '''        C3D_Mtx model{}; camera.writeView(model);
        Mtx_Translate(&model, projectile.x(), projectile.y(), projectile.z(), true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, modelViewUniform_, &model);
        C3D_DrawArrays(GPU_TRIANGLES, static_cast<int>(projectileVertexOffset_), static_cast<int>(projectileVertexCount_));''', '''        C3D_Mtx model{}; camera.writeView(model);
        Mtx_Translate(&model,projectile.x(),projectile.y(),projectile.z(),true);
        const bool rocket=projectile.effect()==ProjectileEffect::GuidedRocket;
        if (rocket) Mtx_RotateY(&model,std::atan2(projectile.velocityX(),projectile.velocityZ()),true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER,modelViewUniform_,&model);
        C3D_DrawArrays(GPU_TRIANGLES,static_cast<int>(rocket?rocketVertexOffset_:projectileVertexOffset_),static_cast<int>(rocket?rocketVertexCount_:projectileVertexCount_));''')
replace('source/Renderer.cpp', '''    levelVertexCount_ = enemyVertexOffset_ = enemyVertexCount_ = 0U;
    towerVertexOffset_ = towerVertexCount_ = projectileVertexOffset_ = projectileVertexCount_ = 0U;''', '''    levelVertexCount_=enemyVertexOffset_=enemyVertexCount_=0U;
    towerVertexOffsets_.fill(0U); towerVertexCounts_.fill(0U);
    projectileVertexOffset_=projectileVertexCount_=0U;
    rocketVertexOffset_=rocketVertexCount_=0U;''')

Path('.github/scripts/issue157_compile_fix.py').unlink()
