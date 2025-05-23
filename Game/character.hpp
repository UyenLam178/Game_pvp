#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include "imgui.h"
#include "animation.hpp" 
#include "texture_manager.hpp" 
#include <vector>
#include <string>
#include "animation.hpp"

class BuffItem;

class Character {
public:
    float x, y;
    float speed;
    ImVec4 color;
    float health;
    float attackRange;
    bool shielded;
    float size;
    float attackDamage;
    bool isDead;
    float attackCooldown, lastAttackTime;
    float skillCooldown, lastSkillTime;
    bool facingRight;
    float spriteWidth;
    float spriteHeight;
    bool isDodging;
    float dodgeCooldown, lastDodgeTime;
    struct DamageNumber {
        float value;
        float x, y;
        float time;
        DamageNumber(float _value, float _x, float _y, float _time) : value(_value), x(_x), y(_y), time(_time) {}
    };
    std::vector<DamageNumber> damageNumbers;

    Character(float _x, float _y, ImVec4 _color, float _health, float _attackDamage);
    virtual ~Character() = default;

    virtual void move(ImGuiKey left, ImGuiKey right, ImGuiKey up, ImGuiKey down);
    virtual void dodge(ImGuiKey dodgeKey);
    virtual void draw() = 0;
    virtual void attack(Character* target, ImGuiKey attackKey = ImGuiKey_E) = 0;
    virtual void useSkill(Character* target) = 0;
    virtual void takeDamage(float damage);
    virtual bool isCollidingWith(Character* other);
    virtual bool isCollidingWith(BuffItem* item);
    virtual bool attackHits(float chanceToHit);
    virtual float randomDamage(float minDamage, float maxDamage, bool& isCrit);
    virtual void applyPushBack(float pushBackX, float pushBackY);
    void updateDamageNumbers(float currentTime);
    void drawDamageNumbers();
};

class Projectile {
public:
    float x, y, velocityX, velocityY, damage;
    bool active;
    ImVec4 color;
    GLuint textureID;

    Projectile(float startX, float startY, float dirX, float dirY, float dmg, ImVec4 col, GLuint texID = 0);
    void update();
    void draw();
};

class DauSi : public Character {
public:
    int comboCount;
    float comboWindow;
    float lastComboTime;
    AnimationController animationController;
    bool isAttacking = false;

    DauSi(float x, float y, ImVec4 color, TextureManager& textureManager);
    void attack(Character* target, ImGuiKey attackKey = ImGuiKey_E) override;
    void useSkill(Character* target) override;
    void draw() override;
};

class XaThu : public Character {
public:
    std::vector<Projectile> projectiles;
    int comboCount;
    float comboWindow;
    float lastComboTime;
    AnimationController animationController;
    float chargeTime;
    TextureManager& textureManager;
    bool isAttacking = false;

    XaThu(float x, float y, ImVec4 color, TextureManager& textureManager);
    void attack(Character* target, ImGuiKey attackKey = ImGuiKey_E) override;
    void useSkill(Character* target) override;
    void draw() override;
};

class BuffItem : public Character {
public:
    enum BuffType { DAMAGE_BOOST, HEAL, SHIELD, SPEED };
    BuffType type;

    BuffItem(float x, float y, ImVec4 color, BuffType t);
    void attack(Character* target, ImGuiKey attackKey = ImGuiKey_E) override {}
    void useSkill(Character* target) override {}
    void applyBuff(Character* ally);
    void draw() override;
    void takeDamage(float damage) override;
};
#endif // CHARACTER_HPP
