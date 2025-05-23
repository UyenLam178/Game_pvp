#include "character.hpp"
#include "imgui.h"
#include <GLFW/glfw3.h>
#include <random>
#include <algorithm>
#include <cstdio>
#include <iostream>

constexpr float CHARACTER_SCALE = 8.0;
const float WINDOW_WIDTH = 1500;
const float WINDOW_HEIGHT = 900;
const float GRAVITY = 0.0;
const float GRASS_BOTTOM = 900.0f - (50.0f * CHARACTER_SCALE);

Character::Character(float _x, float _y, ImVec4 _color, float _health, float _attackDamage)
    : x(_x), y(_y), color(_color), health(_health), attackDamage(_attackDamage), attackRange(50.0f),
    speed(2.0f), isDodging(false), dodgeCooldown(2.0f), lastDodgeTime(0.0f),
    attackCooldown(1.0f), lastAttackTime(0.0f), skillCooldown(5.0f), lastSkillTime(0.0f),
    shielded(false), size(50.0f), isDead(false), facingRight(true) {
}

void Character::move(ImGuiKey left, ImGuiKey right, ImGuiKey up, ImGuiKey down) {
    if (!isDead && !isDodging) {
        if (ImGui::IsKeyDown(left)) {
            x -= speed;
            facingRight = false;
        }
        if (ImGui::IsKeyDown(right)) {
            x += speed;
            facingRight = true;
        }
        if (ImGui::IsKeyDown(up)) {
            y -= speed;
        }
        if (ImGui::IsKeyDown(down)) {
            y += speed;
        }

        float scaledSize = size * CHARACTER_SCALE;
        const float LEFT_LIMIT = -100.0f;
        const float RIGHT_LIMIT = WINDOW_WIDTH - scaledSize + 100.0f;

        x = std::clamp(x, LEFT_LIMIT, RIGHT_LIMIT);
        const float GRASS_TOP = 300.0f;
        const float GRASS_BOTTOM = 900.0f - scaledSize + 89.0f;
        y = std::clamp(y, GRASS_TOP, GRASS_BOTTOM);
    }
}

void Character::dodge(ImGuiKey dodgeKey) {
    float currentTime = static_cast<float>(glfwGetTime());
    if (!isDead && ImGui::IsKeyPressed(dodgeKey) && !isDodging && currentTime - lastDodgeTime > dodgeCooldown) {
        isDodging = true;
        lastDodgeTime = currentTime;
        float dodgeDistance = 50.0f;
        float scaledSize = size * CHARACTER_SCALE;
        if (ImGui::IsKeyDown(ImGuiKey_A) || ImGui::IsKeyDown(ImGuiKey_LeftArrow)) {
            x -= dodgeDistance;
            facingRight = false;
        }
        else if (ImGui::IsKeyDown(ImGuiKey_D) || ImGui::IsKeyDown(ImGuiKey_RightArrow)) {
            x += dodgeDistance;
            facingRight = true;
        }
        x = std::clamp(x, 0.0f, static_cast<float>(WINDOW_WIDTH - scaledSize));
        y = std::clamp(y, 0.0f, static_cast<float>(WINDOW_HEIGHT - scaledSize));
    }
    if (isDodging && currentTime - lastDodgeTime > 0.5f) {
        isDodging = false;
    }
}

void Character::takeDamage(float damage) {
    if (isDead || isDodging) return;
    if (shielded) {
        shielded = false;
        return;
    }
    health -= damage;
    float currentTime = static_cast<float>(glfwGetTime());
    damageNumbers.emplace_back(damage, x + size / 2, y, currentTime);
    if (health <= 0) {
        health = 0;
        isDead = true;
    }
}

void Character::updateDamageNumbers(float currentTime) {
    for (auto it = damageNumbers.begin(); it != damageNumbers.end(); ) {
        it->y -= 1.0f;
        if (currentTime - it->time > 1.0f) {
            it = damageNumbers.erase(it);
        }
        else {
            ++it;
        }
    }
}

void Character::drawDamageNumbers() {
    for (const auto& dn : damageNumbers) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.1f", dn.value);
        ImGui::GetForegroundDrawList()->AddText(ImVec2(dn.x, dn.y), ImColor(1.0f, 1.0f, 1.0f, 1.0f), buffer);
    }
}

bool Character::isCollidingWith(Character* other) {
    if (!other || other->isDead) return false;
    float thisSize = size * CHARACTER_SCALE;
    float otherSize = other->size * CHARACTER_SCALE;

    return x < other->x + otherSize && x + thisSize > other->x &&
        y < other->y + otherSize && y + thisSize > other->y;
}

bool Character::isCollidingWith(BuffItem* item) {
    if (!item) return false;
    float thisSize = size * CHARACTER_SCALE;
    float itemSize = item->size * CHARACTER_SCALE;

    return x < item->x + itemSize && x + thisSize > item->x &&
        y < item->y + itemSize && y + thisSize > item->y;
}

bool Character::attackHits(float chanceToHit) {
    return (rand() % 100) < static_cast<int>(chanceToHit);
}

float Character::randomDamage(float minDamage, float maxDamage, bool& isCrit) {
    float damage = minDamage + (rand() % static_cast<int>(maxDamage - minDamage + 1));
    isCrit = (rand() % 100) < 20;
    if (isCrit) damage *= 2;
    return damage;
}

void Character::applyPushBack(float pushBackX, float pushBackY) {
    x += pushBackX;
    y += pushBackY;
    float scaledSize = size * CHARACTER_SCALE;
    x = std::max(0.0f, std::min(x, static_cast<float>(WINDOW_WIDTH - scaledSize)));
    y = std::max(0.0f, std::min(y, static_cast<float>(WINDOW_HEIGHT - scaledSize)));
}

Projectile::Projectile(float startX, float startY, float dirX, float dirY, float dmg, ImVec4 col, GLuint texID)
    : x(startX), y(startY), velocityX(dirX * 5.0f), velocityY(dirY * 5.0f), damage(dmg), active(true), color(col), textureID(texID) {
    std::cout << "Created projectile at x=" << x << ", y=" << y << ", velocityX=" << velocityX << ", velocityY=" << velocityY << "\n";
}

void Projectile::update() {
    x += velocityX;
    velocityY += GRAVITY;
    y += velocityY;
    if (x < 0 || x > WINDOW_WIDTH || y < 0 || y > WINDOW_HEIGHT) {
        active = false;
        std::cout << "Projectile deactivated at x=" << x << ", y=" << y << "\n";
    }
}

void Projectile::draw() {
    if (active) {
        if (textureID != 0) {
            ImVec2 topLeft(x, y);
            ImVec2 bottomRight(x + 50, y + 30);
            ImGui::GetForegroundDrawList()->AddImage(
                (ImTextureID)(intptr_t)textureID, topLeft, bottomRight, ImVec2(0, 0), ImVec2(1, 1));
        }
        else {
            ImGui::GetForegroundDrawList()->AddRectFilled(
                ImVec2(x, y), ImVec2(x + 10, y + 5), ImGui::GetColorU32(color));
        }
        std::cout << "Drawing projectile at x=" << x << ", y=" << y << "\n";
    }
}

DauSi::DauSi(float x, float y, ImVec4 color, TextureManager& textureManager)
    : Character(x, y, color, 150.0f, 25.0f), // dùng x truyền vào đúng
    comboCount(0), comboWindow(1.0f), lastComboTime(0.0f), animationController(textureManager)
{
    speed = 4.0f;
    attackRange = 40.0f;
    attackCooldown = 0.5f;
    skillCooldown = 8.0f;

    // Đặt hướng mặt dựa vào vị trí
    facingRight = (x < WINDOW_WIDTH / 2.0f);

    // Load texture
    textureManager.loadTexture("dausi_idle", "../x64/Debug/DauSi/Sprites/Idle.png");
    textureManager.loadTexture("dausi_run", "../x64/Debug/DauSi/Sprites/Run.png");
    textureManager.loadTexture("dausi_attack", "../x64/Debug/DauSi/Sprites/Attack1.png");

    animationController.addAnimation("idle", { "dausi_idle" }, 0.1f, true, 10, true);
    animationController.addAnimation("run", { "dausi_run" }, 0.1f, true, 6, true);
    animationController.addAnimation("attack", { "dausi_attack" }, 0.01f, false, 4, true);
}


void DauSi::attack(Character* target, ImGuiKey attackKey) {
    float currentTime = static_cast<float>(glfwGetTime());
    if (target && ImGui::IsKeyPressed(attackKey) && currentTime - lastAttackTime > attackCooldown && !isDodging) {
        animationController.playAnimation("attack", currentTime);
        isAttacking = true;
        if (isCollidingWith(target)) {
            if (currentTime - lastComboTime < comboWindow) {
                comboCount++;
            }
            else {
                comboCount = 1;
            }
            lastComboTime = lastAttackTime = currentTime;
            bool isCrit = false;
            float damage = randomDamage(15.0f, 20.0f, isCrit);
            if (comboCount >= 3) {
                damage *= 1.5f;
                target->applyPushBack((target->x > x) ? 30.0f : -30.0f, -20.0f);
                comboCount = 0;
            }
            if (attackHits(85.0f)) {
                target->takeDamage(damage);
            }
        }
    }
    animationController.update(currentTime);
}

void DauSi::useSkill(Character* target) {
    float currentTime = static_cast<float>(glfwGetTime());
    if (ImGui::IsKeyPressed(ImGuiKey_Q) && currentTime - lastSkillTime > skillCooldown && !isDodging) {
        lastSkillTime = currentTime;
        float chargeDistance = 100.0f;
        if (target && target->x > x) {
            x += chargeDistance;
            facingRight = true;
        }
        else {
            x -= chargeDistance;
            facingRight = false;
        }
        x = std::max(0.0f, std::min(x, WINDOW_WIDTH - size * CHARACTER_SCALE));
        if (target && isCollidingWith(target)) {
            target->applyPushBack((target->x > x) ? 50.0f : -50.0f, 0);
            target->takeDamage(30.0f);
        }
    }
    animationController.update(currentTime);
}

void DauSi::draw() {
    float currentTime = static_cast<float>(glfwGetTime());

    // Check phím điều khiển theo màu
    bool isMoving = false;
    if (color.x == 1.0f && color.y == 0.0f) { // Player 1
        isMoving = ImGui::IsKeyDown(ImGuiKey_A) || ImGui::IsKeyDown(ImGuiKey_D) ||
            ImGui::IsKeyDown(ImGuiKey_W) || ImGui::IsKeyDown(ImGuiKey_S);
    }
    else {
        isMoving = ImGui::IsKeyDown(ImGuiKey_LeftArrow) || ImGui::IsKeyDown(ImGuiKey_RightArrow) ||
            ImGui::IsKeyDown(ImGuiKey_UpArrow) || ImGui::IsKeyDown(ImGuiKey_DownArrow);
    }

    if (isAttacking && animationController.hasFinished("attack", currentTime)) {
        isAttacking = false;
    }

    if (!isAttacking) {
        if (isMoving)
            animationController.playAnimation("run", currentTime);
        else
            animationController.playAnimation("idle", currentTime);
    }

    animationController.update(currentTime);
    FrameResult frame = animationController.getCurrentFrame();

    ImVec2 topLeft(x, y);
    ImVec2 bottomRight(x + size * CHARACTER_SCALE, y + size * CHARACTER_SCALE);

    if (frame.textureID != 0) {
        ImVec2 uv0 = facingRight ? frame.uv0 : ImVec2(frame.uv1.x, frame.uv0.y);
        ImVec2 uv1 = facingRight ? frame.uv1 : ImVec2(frame.uv0.x, frame.uv1.y);
        ImVec2 flippedUV0 = ImVec2(uv0.x, uv1.y);
        ImVec2 flippedUV1 = ImVec2(uv1.x, uv0.y);

        ImGui::GetForegroundDrawList()->AddImage(
            (ImTextureID)(intptr_t)frame.textureID, topLeft, bottomRight, flippedUV0, flippedUV1);
    }
    else {
        ImGui::GetForegroundDrawList()->AddRectFilled(topLeft, bottomRight, ImColor(color));
    }
}


XaThu::XaThu(float x, float y, ImVec4 color, TextureManager& tm)
    : Character(x, y, color, 120.0f, 15.0f),
      comboCount(0), comboWindow(1.0f), lastComboTime(0.0f),
      animationController(tm), chargeTime(0.0f), textureManager(tm)
{
    speed = 4.5f;
    attackRange = 200.0f;
    attackCooldown = 3.0f;
    skillCooldown = 6.0f;

    // Đặt hướng mặt dựa vào vị trí
    facingRight = (x < WINDOW_WIDTH / 2.0f);

    // Load textures
    textureManager.loadTexture("xathu_idle", "../x64/Debug/XaThu/Idle.png");
    textureManager.loadTexture("xathu_running", "../x64/Debug/XaThu/Running.png");
    textureManager.loadTexture("xathu_attack", "../x64/Debug/XaThu/Attack.png");
    textureManager.loadTexture("arrow", "../x64/Debug/XaThu/arrow.png");

    animationController.addAnimation("run", { "xathu_running" }, 0.1f, true, 8, true);
    animationController.addAnimation("idle", { "xathu_idle" }, 0.1f, true, 8, true);
}


void XaThu::attack(Character* target, ImGuiKey attackKey) {
    float currentTime = static_cast<float>(glfwGetTime());
    if (ImGui::IsKeyDown(attackKey)) {
        chargeTime += ImGui::GetIO().DeltaTime;
        if (chargeTime > 5.0f) chargeTime = 5.0f;
        animationController.playAnimation("attack", currentTime);
        isAttacking = true;
    }
    if (ImGui::IsKeyReleased(attackKey) && chargeTime > 0.1f && currentTime - lastAttackTime >= attackCooldown && !isDodging) {
        std::cout << "XaThu attacking, creating projectile with chargeTime: " << chargeTime << "\n";
        lastAttackTime = currentTime;
        if (currentTime - lastComboTime < comboWindow) {
            comboCount++;
        }
        else {
            comboCount = 1;
        }
        lastComboTime = currentTime;
        bool isCrit = false;
        float damage = randomDamage(10.0f, 15.0f, isCrit);
        if (comboCount == 3) {
            damage *= 1.5f;
            comboCount = 0;
        }
        float dirX = 0.0f;
        float dirY = 0.0f;
        float chargeFactor = std::min(chargeTime / 5.0f, 1.0f) * 2.0f + 1.0f;
        if (attackKey == ImGuiKey_E || attackKey == ImGuiKey_Space) {
            dirX = (target->x > x) ? 1.0f : -1.0f;
            dirY = 0.0f;
            facingRight = (dirX > 0);
        }
        else if (attackKey == ImGuiKey_Q) {
            dirX = 0.0f;
            dirY = -2.0f;
            facingRight = true;
        }
        float projectileX = x + (facingRight ? size * CHARACTER_SCALE : 0.0f);
        float projectileY = y + (size * CHARACTER_SCALE * 0.5f);
        GLuint arrowTexture = textureManager.getTexture("arrow");
        projectiles.emplace_back(projectileX, projectileY, dirX * chargeFactor, dirY * chargeFactor, damage, color, arrowTexture);
        chargeTime = 0.0f;
        std::cout << "Projectile created, total projectiles: " << projectiles.size() << "\n";
    }
    for (auto it = projectiles.begin(); it != projectiles.end(); ) {
        it->update();

        float targetSize = target ? target->size * CHARACTER_SCALE : 0.0f;
        float targetMidTop = target->y + targetSize * 0.4f; // 40% chiều cao
        float targetMidBottom = target->y + targetSize * 0.6f; // 60% chiều cao

        if (it->active && target && it->x >= target->x && it->x <= target->x + targetSize &&
            it->y >= targetMidTop && it->y <= targetMidBottom) {
            if (attackHits(90.0f)) {
                target->takeDamage(it->damage);
                std::cout << "Projectile hit target at midsection, damage: " << it->damage << "\n";
            }
            it->active = false;
        }

        if (!it->active) {
            it = projectiles.erase(it);
            std::cout << "Projectile removed, remaining: " << projectiles.size() << "\n";
        }
        else {
            ++it;
        }
    }
    animationController.update(currentTime);
}

void XaThu::useSkill(Character* target) {
    float currentTime = static_cast<float>(glfwGetTime());
    if (ImGui::IsKeyPressed(ImGuiKey_Q) && currentTime - lastSkillTime > skillCooldown && !isDodging) {
        lastSkillTime = currentTime;
        float dirX = (target && target->x > x) ? 1.0f : -1.0f;
        facingRight = (dirX > 0);
        float projectileY = y + (size * CHARACTER_SCALE * 0.75f);
        projectiles.emplace_back(x + size / 2, projectileY, dirX, 0.0f, 30.0f, ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
            textureManager.getTexture("arrow"));
        std::cout << "XaThu used skill, created special projectile\n";
    }
    animationController.update(currentTime);
}

void XaThu::draw() {
    float currentTime = static_cast<float>(glfwGetTime());

    bool isMoving = false;
    if (color.x == 1.0f && color.y == 0.0f) {
        isMoving = ImGui::IsKeyDown(ImGuiKey_A) || ImGui::IsKeyDown(ImGuiKey_D) ||
            ImGui::IsKeyDown(ImGuiKey_W) || ImGui::IsKeyDown(ImGuiKey_S);
    }
    else {
        isMoving = ImGui::IsKeyDown(ImGuiKey_LeftArrow) || ImGui::IsKeyDown(ImGuiKey_RightArrow) ||
            ImGui::IsKeyDown(ImGuiKey_UpArrow) || ImGui::IsKeyDown(ImGuiKey_DownArrow);
    }

    if (isAttacking && animationController.hasFinished("attack", currentTime)) {
        isAttacking = false;
    }

    if (!isAttacking) {
        if (isMoving)
            animationController.playAnimation("run", currentTime);
        else
            animationController.playAnimation("idle", currentTime);
    }

    animationController.update(currentTime);
    FrameResult frame = animationController.getCurrentFrame();

    float textureWidth = 512.0f;
    float textureHeight = 64.0f;

    float uvWidth = frame.uv1.x - frame.uv0.x;
    float uvHeight = frame.uv1.y - frame.uv0.y;

    float frameWidth = textureWidth * uvWidth;
    float frameHeight = textureHeight * uvHeight;

    float spriteWidth = frameWidth * CHARACTER_SCALE;
    float spriteHeight = frameHeight * CHARACTER_SCALE;
    ImVec2 topLeft(x, y);
    ImVec2 bottomRight(x + spriteWidth, y + spriteHeight);

    if (frame.textureID != 0) {
        ImVec2 uv0 = facingRight ? frame.uv0 : ImVec2(frame.uv1.x, frame.uv0.y);
        ImVec2 uv1 = facingRight ? frame.uv1 : ImVec2(frame.uv0.x, frame.uv1.y);
        ImVec2 flippedUV0 = ImVec2(uv0.x, uv1.y);
        ImVec2 flippedUV1 = ImVec2(uv1.x, uv0.y);

        ImGui::GetForegroundDrawList()->AddImage(
            (ImTextureID)(intptr_t)frame.textureID, topLeft, bottomRight, flippedUV0, flippedUV1);
    }
    else {
        ImGui::GetForegroundDrawList()->AddRectFilled(topLeft, bottomRight, ImColor(color));
    }

    for (auto& p : projectiles) {
        p.draw();
    }
}


BuffItem::BuffItem(float x, float y, ImVec4 color, BuffType t)
    : Character(x, y, color, 50.0f, 0.0f), type(t) {
    speed = 0.0f;
    size = 20.0f;
}

void BuffItem::applyBuff(Character* ally) {
    if (!ally || ally->isDodging) return;
    switch (type) {
    case DAMAGE_BOOST:
        ally->attackDamage += 5.0f;
        break;
    case HEAL:
        ally->health += 20.0f;
        if (ally->health > (dynamic_cast<DauSi*>(ally) ? 150.0f : 120.0f)) {
            ally->health = dynamic_cast<DauSi*>(ally) ? 150.0f : 120.0f;
        }
        break;
    case SHIELD:
        ally->shielded = true;
        break;
    case SPEED:
        ally->speed += 1.0f;
        break;
    }
}

void BuffItem::draw() {
    ImVec2 topLeft(x, y);
    ImVec2 bottomRight(x + size, y + size);
    ImGui::GetForegroundDrawList()->AddRectFilled(topLeft, bottomRight, ImColor(color));
}

void BuffItem::takeDamage(float damage) {
}